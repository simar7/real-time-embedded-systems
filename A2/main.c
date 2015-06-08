#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_ms;
uint32_t ms_coeff = 1000;
uint32_t sec_in_min = 60;
uint32_t ms_in_min = 60000;
uint32_t min_to_run = 10;

// States
#define S0 	0		// START
#define S1	1
#define S2	2
#define S3	3
#define S4	4
#define S5	5
#define S6	6
#define S7	7		// COMPLETE

// Events
#define DOT 0
#define DASH 1

// Transition Table
// t_table[event][curstate]
int t_table[][] = {
	// S0		S1		S2		S3		S4		S5		S6
	{ S1, S0, S1, S4, S1, S6, S7},	// DOT
	{ S0, S2, S3, S0, S5, S3, S2},	// DASH
};

// State Machine
enum FSMSTATE = { 
S0, S1, S2, S3, S4, S5, S6, S7
} curr_state;

// Initial State
curr_state = S0;

struct FSM {
	FSMSTATE curr_state;
	int nextState = S0;
}

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
    GLCD_Clear(Black);
		GLCD_SetTextColor(Red);
		GLCD_SetBackColor(Black);
		__enable_irq();
	
		GLCD_DisplayString(1, 1, 1, "INPUT: ");
		GLCD_DisplayString(1, 9, 1, "DOT");
		GLCD_DisplayString(4, 5, 1, "TIMER");
		BoardInit();
	
		hwInterrupt(min_to_run * ms_in_min);
		GLCD_DisplayString(1, 9, 1, "END  ");
    return 0;
}
