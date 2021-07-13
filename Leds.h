/*
 * Leds.h
 *
 *  Created on: 14 Haz 2020
 *      Author: POLAT B
 */

#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#endif /* INC_LEDS_H_ */

#include "main.h"

#define  NUMLEDS 20
#define  LED_CFG_BYTES_PER_LED 3

uint8_t pwmbuffer[48];

enum LedStatus{
		Free,
		Updating
};


uint32_t leds_colors[LED_CFG_BYTES_PER_LED*NUMLEDS];

uint8_t currentled;
uint8_t tc;
enum LedStatus ledStatus;

void ws2812Init(void);
void led_update(void);
void DMA1_Channel2_3_IRQHandler(void);
void ws2812_PWM_Start(void);
void ws2812_PWM_Stop(void);
uint8_t led_fill_led_pwm_data(void);
uint8_t led_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
uint8_t led_set_color_all(uint8_t r, uint8_t g, uint8_t b);
int led_fill_pwm_buffer(int led,int buf);
void led_shift(void);

