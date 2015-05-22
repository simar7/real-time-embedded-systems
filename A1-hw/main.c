#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_ms;
uint32_t ms_coeff = 1000;
uint32_t sec_in_min = 60;
uint32_t ms_in_min = 60000;
uint32_t min_to_run = 10;

void BoardInit() {
	g_ms = 0;
	LPC_TIM0->TCR = 0x02;	// reset timer
	LPC_TIM0->TCR = 0x01; // enable timer
	LPC_TIM0->MR0 = 25000; // match value (10 minute mark)
	LPC_TIM0->MCR |= 0x03; // on match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER0_IRQn); // enable timer interrupts
}

void hwInterrupt(uint32_t milliseconds) {
	char cur_time[128];

	while (g_ms <= milliseconds+ms_coeff) {
		sprintf(cur_time, "%02d:%02d", ((g_ms)/ms_in_min),((g_ms)/ms_coeff)%sec_in_min);
		GLCD_DisplayString(5, 5, 1, (unsigned char *)cur_time);
	}
}

void TIMER0_IRQHandler() {
		LPC_TIM0->IR |= 0x01;	// clear the interrupt.
		//GLCD_DisplayString(9, 1, 1, "Interrupt hit")
		g_ms++;
}


int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
		__enable_irq();
		GLCD_DisplayString(1, 1, 1, "Status: ");
		GLCD_DisplayString(1, 9, 1, "START");
		GLCD_DisplayString(4, 5, 1, "TIMER");
		BoardInit();
		hwInterrupt(min_to_run * ms_in_min);
		GLCD_DisplayString(1, 9, 1, "END  ");
    return 0;
}
