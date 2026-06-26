#ifndef INC_ENCODER_SSI_H_
#define INC_ENCODER_SSI_H_

#include "main.h"
#include <stdint.h>

#define LA11SC_DATA_BITS          26U
#define LA11SC_SPI_RX_WORD_COUNT  2U
#define LA11SC_POS26_MASK         0x03FFFFFFU

typedef struct
{
  char type;
  uint32_t resolution;
  uint32_t tolerance;
  uint32_t home;
  uint32_t lim_up;
  uint32_t lim_down;

  volatile uint8_t state;
  volatile uint8_t data_ready;
  volatile uint8_t request_read;
  volatile uint8_t is_busy;

  uint16_t rx_words[LA11SC_SPI_RX_WORD_COUNT];
  uint16_t tx_words[LA11SC_SPI_RX_WORD_COUNT];

  volatile uint32_t raw32;
  volatile uint32_t position;
  volatile uint32_t turn;
  volatile uint32_t gray;
  volatile uint32_t bin;

} la11sc_spi_t;

extern la11sc_spi_t la11sc_encoder;

uint8_t la11sc_spi_cnt_size(void);
void la11sc_spi_start(la11sc_spi_t *encoder);
void la11sc_spi_request_read(la11sc_spi_t *encoder);
void la11sc_spi_read(la11sc_spi_t *encoder, SPI_HandleTypeDef *hspi);
void la11sc_spi_irq(la11sc_spi_t *encoder);
void la11sc_spi_error(la11sc_spi_t *encoder);

#endif /* INC_ENCODER_SSI_H_ */
