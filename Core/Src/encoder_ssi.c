#include "encoder_ssi.h"

la11sc_spi_t la11sc_encoder = {0};

static void la11sc_spi_dat(la11sc_spi_t *encoder)
{
  uint32_t pos26;

  encoder->raw32 = ((uint32_t)encoder->rx_words[0] << 16) |
                   (uint32_t)encoder->rx_words[1];
  pos26 = (encoder->raw32 >> (32U - LA11SC_DATA_BITS)) & LA11SC_POS26_MASK;
  encoder->position = pos26 >> 2;
  encoder->data_ready = 1U;
  encoder->is_busy = 0U;
}

void la11sc_spi_start(la11sc_spi_t *encoder)
{
  encoder->type = 'l';
  encoder->resolution = 16777216U;
  if (encoder->tolerance == 0U)
  {
    encoder->tolerance = 20U;
  }
  encoder->home = 0U;
  encoder->lim_up = 16777216U;
  encoder->lim_down = 0U;

  encoder->rx_words[0] = 0U;
  encoder->rx_words[1] = 0U;
  encoder->tx_words[0] = 0U;
  encoder->tx_words[1] = 0U;

  encoder->raw32 = 0U;
  encoder->position = 0U;

  encoder->state = 1U;
  encoder->data_ready = 0U;
  encoder->request_read = 0U;
  encoder->is_busy = 0U;
}

void la11sc_spi_read(la11sc_spi_t *encoder, SPI_HandleTypeDef *hspi)
{
  if (encoder->is_busy)
  {
    return;
  }

  encoder->is_busy = 1U;
  encoder->request_read = 0U;
  encoder->data_ready = 0U;

  encoder->rx_words[0] = 0U;
  encoder->rx_words[1] = 0U;

  if (HAL_SPI_TransmitReceive_DMA(hspi,
                                  (uint8_t *)encoder->tx_words,
                                  (uint8_t *)encoder->rx_words,
                                  LA11SC_SPI_RX_WORD_COUNT) != HAL_OK)
  {
    la11sc_spi_error(encoder);
  }
}

void la11sc_spi_irq(la11sc_spi_t *encoder)
{
  la11sc_spi_dat(encoder);
}

void la11sc_spi_error(la11sc_spi_t *encoder)
{
  encoder->is_busy = 0U;
}

void la11sc_spi_request_read(la11sc_spi_t *encoder)
{
  encoder->request_read = 1U;
}

uint8_t la11sc_spi_cnt_size(void)
{
  return LA11SC_DATA_BITS;
}
