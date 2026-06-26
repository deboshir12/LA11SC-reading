/*
 * la11sc.h
 *
 *  Created on: 9 июл. 2024 г.
 *      Author: -
 */

#ifndef SRC_DRV_LA11SC_H_
#define SRC_DRV_LA11SC_H_

#include "enc.h"

uint8_t la11sc_cnt_size(void);
void la11sc_start(mc_encoder_t *encoder);
void la11sc_irq(mc_encoder_t *encoder);

#endif /* SRC_DRV_LA11SC_H_ */
