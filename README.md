# la11sc SSI encoder reading

Проект читает линейный энкодер LA11SC через SPI1 в режиме master. Плата сама
выдает такты SPI. Чтение запускается периодически от TIM10.

За один SPI DMA transfer принимаются два 16-битных слова, то есть 32 бита. У
LA11SC полезные данные занимают 26 бит, а конец 32-битного чтения добивается
нулями. Поэтому сначала из `raw32` убираются 6 младших хвостовых битов, затем
из 26-битного значения убираются 2 служебных младших бита формата LA11 2D0.

```c
pos26 = (encoder->raw32 >> (32U - LA11SC_DATA_BITS)) & LA11SC_POS26_MASK;
encoder->position = pos26 >> 2;
```

После этого `encoder->position` хранит позицию в микрометрах.

## Где Лежит Логика

- `Core/Inc/encoder_ssi.h` - интерфейс библиотеки и структура состояния
  `la11sc_spi_t`.
- `Core/Src/encoder_ssi.c` - чтение LA11SC через SPI DMA и преобразование
  принятого значения.
- `Core/Src/main.c` - инициализация SPI/DMA/TIM и вызовы библиотеки из main loop
  и HAL callbacks.
- `Core/Src/stm32f7xx_hal_msp.c` - привязка SPI1 к DMA и включение TIM10 IRQ.
- `Core/Src/stm32f7xx_it.c` - обработчики TIM10, SPI1 и DMA IRQ.

## Состояние Энкодера

Глобальный объект состояния:

```c
la11sc_spi_t la11sc_encoder;
```

Основные поля:

- `request_read` - запрос нового чтения. Ставится из TIM10 callback через
  `la11sc_spi_request_read()`, сбрасывается перед запуском SPI DMA.
- `is_busy` - признак активного SPI DMA чтения. Не дает запустить новое чтение,
  пока предыдущее не завершилось.
- `data_ready` - признак, что новое значение принято и обработано.
- `rx_words[2]` - два принятых 16-битных SPI слова.
- `tx_words[2]` - два передаваемых 16-битных слова, нужны SPI master для
  генерации 32 тактов.
- `raw32` - полное 32-битное значение, собранное из `rx_words`.
- `position` - готовая позиция в микрометрах.

Поля `turn`, `gray`, `bin` оставлены в структуре, но текущая SPI-реализация их
не использует.

## Константы

- `LA11SC_DATA_BITS = 26U` - количество полезных бит LA11SC.
- `LA11SC_SPI_RX_WORD_COUNT = 2U` - два 16-битных слова на одно чтение.
- `LA11SC_POS26_MASK = 0x03FFFFFFU` - маска для 26-битного значения.

## Настройки HAL

В `Core/Inc/stm32f7xx_hal_conf.h` должны быть включены:

- `HAL_SPI_MODULE_ENABLED`
- `HAL_TIM_MODULE_ENABLED`
- `HAL_DMA_MODULE_ENABLED`

Compiler defines:

- `USE_HAL_DRIVER`
- `STM32F767xx`

## Настройки Периферии

SPI1:

- `SPI_MODE_MASTER`
- `SPI_DIRECTION_2LINES`
- `SPI_DATASIZE_16BIT`
- `SPI_POLARITY_HIGH`
- `SPI_PHASE_2EDGE`
- `SPI_NSS_SOFT`
- `SPI_BAUDRATEPRESCALER_64`
- Пины: `PA5` SCK, `PA6` MISO, `PA7` MOSI.

DMA:

- SPI1 RX: `DMA2_Stream0`, `DMA_CHANNEL_3`, halfword alignment.
- SPI1 TX: `DMA2_Stream3`, `DMA_CHANNEL_3`, halfword alignment.

TIM10:

- Используется как периодический триггер чтения.
- `Prescaler = 160`
- `Period = 1000`
- IRQ: `TIM1_UP_TIM10_IRQn`

NVIC:

- `DMA2_Stream0_IRQn`
- `DMA2_Stream3_IRQn`
- `SPI1_IRQn`
- `TIM1_UP_TIM10_IRQn`

## Поток Выполнения

1. `HAL_TIM_PeriodElapsedCallback()` срабатывает от TIM10.
2. Callback вызывает `la11sc_spi_request_read(&la11sc_encoder)`.
3. В `while (1)` проверяется `la11sc_encoder.request_read`.
4. `la11sc_spi_read(&la11sc_encoder, &hspi1)` запускает
   `HAL_SPI_TransmitReceive_DMA()`.
5. SPI1 master выдает 32 SCK такта и принимает два 16-битных слова.
6. `HAL_SPI_TxRxCpltCallback()` вызывает `la11sc_spi_irq(&la11sc_encoder)`.
7. `la11sc_spi_irq()` собирает `raw32`, выделяет 26 бит и обновляет
   `la11sc_encoder.position`.
8. Если запуск SPI DMA завершился ошибкой или пришел SPI error callback,
   `la11sc_spi_error()` сбрасывает `is_busy`.
