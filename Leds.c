/*
 * Leds.c
 *
 *  Created on: 14 Haz 2020
 *      Author: POLAT B
 */
#include "Leds.h"
#include "string.h"

int led_fill_pwm_buffer(int led,int buf){
	    for (size_t i = 0; i < 8; i++) {
	        	pwmbuffer[24*buf+i]=  (leds_colors[LED_CFG_BYTES_PER_LED * led + 1] & (1 << (7 - i))) ? (43) : (17);//34 17
	        	pwmbuffer[24*buf+i+8] =  (leds_colors[LED_CFG_BYTES_PER_LED * led + 0] & (1 << (7 - i))) ? (43) : (17);
	        	pwmbuffer[24*buf+i+16] =  (leds_colors[LED_CFG_BYTES_PER_LED * led + 2] & (1 << (7 - i))) ? (43) : (17);
	    }
	   return 1;
}
uint8_t led_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
	while(ledStatus!=Free);
	ledStatus=Updating;
    if (index < NUMLEDS) {
        leds_colors[index * LED_CFG_BYTES_PER_LED + 0] = r;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 1] = g;
        leds_colors[index * LED_CFG_BYTES_PER_LED + 2] = b;
        ledStatus=Free;
        return 1;
    }
    ledStatus=Free;
    return 0;
}
uint8_t led_set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (size_t index = 0; index < NUMLEDS; index++) {
        led_set_color(index,r,g,b);
    }
    return 1;
}
void ws2812_PWM_Stop(void){
	TIM16->BDTR&=~(1<<15);//output disable
	TIM16->CR1&=~1;//counter disable
	TIM16->CCER&=~1;//cc output disable
	DMA1_Channel3->CCR&=~1;// dma channel disable(1<<2)(1<<3)(1<<1)
}
void ws2812_PWM_Start(void){
	TIM16->CCER|=1;//cc output enable
	TIM16->CR1|=1;// counter enable
	DMA1_Channel3->CCR|=1;
	TIM16->BDTR|=(1<<15);// main output enable
}
void led_update(void){
	while(ledStatus!=Free);
	ledStatus=Updating;
	currentled=0;
	tc=1;
	memset(pwmbuffer,0,sizeof(pwmbuffer));//start pulse
	ws2812_PWM_Start();
}

void ws2812Init(void){
	RCC->AHBENR|=RCC_AHBENR_GPIOAEN;// a port open
	RCC->AHBENR|=1; //dma clock en
	RCC->APB2ENR|=(1<<17);// tim clock enable

	GPIOA->MODER|=(1<<13);
	GPIOA->OSPEEDR|=(1<<12)|(1<<13);
	GPIOA->AFR[0]|=(1<<26)|(1<<24);

	TIM16->PSC=0;
	TIM16->ARR=59;
	TIM16->CR1|=(1<<7);// auto preload enable//
	TIM16->DIER|=(1<<9); //cc1 dma req enable
	TIM16->CCMR1|=(1<<6)|(1<<5); // pwm mode 1
	TIM16->CCMR1|=(1<<3);// preload enable
	TIM16->CCER|=1;//cc output enable
	TIM16->DMAR|=1;// ****************?? denencek

	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
	NVIC_SetPriority(DMA1_Channel2_3_IRQn,1);
	DMA1_Channel3->CPAR=(uint32_t)(&(TIM16->CCR1));
	DMA1_Channel3->CMAR=(uint32_t)pwmbuffer;
	DMA1_Channel3->CNDTR=sizeof(pwmbuffer);
	DMA1_Channel3->CCR|=(1<<1)|(1<<3)|(1<<4)|(1<<5)|(1<<7)|(1<<9)|(1<<2);// tr comp it,tr err it,data trans direction,circ mod,mem inc,TIM16->EGR|=(1<<1); // cc1 generation
	ledStatus=Free;
}

void DMA1_Channel2_3_IRQHandler(void){
	if(DMA1->ISR & (1<<9)){// tc event control
		DMA1->IFCR|=(1<<9);// clear flag
		if(currentled<=NUMLEDS){
				tc=1;
				led_fill_pwm_buffer(currentled,tc);
				currentled++;
			}
			else{
				ws2812_PWM_Stop();
				ledStatus=Free;
			}
	}
	if(DMA1->ISR & (1<<10)){// ht control
		DMA1->IFCR|=(1<<10); // clear flag
		if(currentled<=NUMLEDS){
				tc=0;
				led_fill_pwm_buffer(currentled,tc);
				currentled++;
			}
		else{
			ws2812_PWM_Stop();
			ledStatus=Free;
		}

	}
	if(DMA1->ISR  & (1<<11)){ // tr error control
		DMA1->IFCR|=(1<<11); // clear flag
	}
}
void led_shift(void){
	for(int i=LED_CFG_BYTES_PER_LED*NUMLEDS-3;i>=0;i--){
			leds_colors[i+3]=leds_colors[i];
		}
}



