/*
 * la11sc.c
 *
 *  Created on: 9 июл. 2024 г.
 *      Author: -
 */


#include "la11sc.h"
#include <stdint.h>

#define DATA_BITS 26
#define END_CYCLE DATA_BITS*2 + 1

static void la11sc_dat(mc_encoder_t *Encoder);
extern TIM_HandleTypeDef htim1;
uint8_t la11sc_cnt_size(void)
{
	return DATA_BITS;
}

void la11sc_start(mc_encoder_t *encoder)
{
	encoder->type 					= 		'l';
	encoder->resolution				=		16777216;
	if(encoder->tolerance == 0)
		encoder->tolerance			=		20;//100;
	encoder->home					=		0;
	encoder->lim_up					=		16777216;
	encoder->lim_down				=		0;
//	encoder->CLK_Port				=		RS485_I1_GPIO_Port;
//	encoder->CLK_Pin				=		RS485_I1_Pin;
	encoder->DATA_Port				=		RS485_O2_GPIO_Port;
	encoder->DATA_Pin				=		RS485_O2_Pin;
	encoder->state = 1;
	encoder->cnt = 1;
//	HAL_GPIO_WritePin(encoder->CLK_Port, encoder->CLK_Pin, GPIO_PIN_SET);
}

static void la11sc_dat(mc_encoder_t *Encoder)
{
	Encoder->position = 0;
	Encoder->turn = 0;

#ifdef ENC_HARD_ERR
	Encoder->gray = MASK_GRAY_S_18 & (~Encoder->gray); 	//TODO: костыль
#endif

//	Encoder->bin = Encoder->gray;
	Encoder->position = Encoder->bin>>2;
	Encoder->gray = 0;
	Encoder->bin = 0;
	Encoder->data_ready = 1;
}

void la11sc_irq(mc_encoder_t *encoder)
{
	if(encoder->cnt == 0)
	{
		htim1.Instance->ARR = 4;
	}
	if(encoder->cnt%2==0&&encoder->cnt>0) //even values
	{
		encoder->bin = encoder->bin<<1;
		encoder->bin |= HAL_GPIO_ReadPin(encoder->DATA_Port, encoder->DATA_Pin);

	}
	if(encoder->cnt == END_CYCLE)
	{
		htim1.Instance->ARR = 49;
		la11sc_dat(encoder);
		encoder->cnt = 0;
		return;
	}
	encoder->cnt++;

}
