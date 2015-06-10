#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t ms_coeff = 1000;
uint32_t sec_in_min = 60;
uint32_t ms_in_min = 60000;
uint32_t min_to_run = 10;
uint32_t magic_number = 16550;
uint32_t i = 0;
uint32_t j = 1;
uint32_t milliseconds = 100;

char curr_state[16];
volatile int input;
volatile uint32_t global_time;
volatile char counter[64];
int TS = 3000000;
int timeSpike = 10000;
volatile int persist_dot = 1;
volatile int persist_dash = 1;
volatile int prev_button_val = 0;
volatile int global_button_input = 0;

#define MAX_PERSIST 3
#define TOTAL_STATES 8
#define TOTAL_EVENTS 2

typedef enum { S0, S1, S2, S3, S4, S5, S6, S7 } state_t;
typedef enum { DOT, DASH } event_t;
	
state_t transition_matrix[TOTAL_STATES][TOTAL_EVENTS] = {
	{S1, S0},	// S0
	{S0, S2}, // S1
	{S1, S3}, // S2
	{S4, S0}, // S3
	{S1, S5}, // S4
	{S6, S3}, // S5
	{S7, S2}, // S6
	{S7, S7}  // S7
};

void init_system()
{
	// Initialize system and GLCD
	SystemInit();
	GLCD_Init();
	GLCD_Clear(White);
}

void init_int0()
{
	// Set INT0 as input
	LPC_PINCON->PINSEL4 &= ~(3<<20);		// P2.10 is GPIO
	LPC_GPIO2->FIODIR		&= ~(1<<1);		// P2.10 is input
	
	LPC_GPIOINT->IO2IntEnF |= 1 << 10; // falling edge of P2.10		BUTTON IS PRESSED
	LPC_GPIOINT->IO2IntEnR |= 1 << 10; // rising edge of P2.10	BUTTON IS RELEASED
	NVIC_EnableIRQ(EINT3_IRQn);
}


void EINT3_IRQHandler()
{
	int int0_val;
	volatile int tim0_val;
	
	// Clear the interrupt on EINT0
	LPC_GPIOINT->IO2IntClr |= 1 << 10;
	
	int0_val =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01;
	//tim0_val = LPC_TIM0->TC;

	if (int0_val == 0)		// Button is pressed
	{
		// Start Timer
	  LPC_TIM0->TCR = 0x02;
		LPC_TIM0->TCR = 0x01;
		LPC_TIM0->PR  = 0x00; 
	}
	else if (int0_val == 1)		// Button is released
	{
		tim0_val = LPC_TIM0->TC;
		// Stop Timer
		LPC_TIM0->TCR = 0x02;
	}

	global_time = tim0_val;
	
	if (global_time >= 9999999 && int0_val == 1) {
		persist_dash += 1;
		global_button_input = 1;
	} else if(global_time < 9999999 && int0_val == 0) {
		persist_dot += 1;
		global_button_input = 0;
	}
	
}

void checkIfDotOrDash() {
	if (persist_dash == MAX_PERSIST) {		// DASH
		global_button_input = 1;
		prev_button_val = global_button_input;
		persist_dash = 0;
	} else if(persist_dot == MAX_PERSIST) {												// DOT
		global_button_input = 0;
		prev_button_val = global_button_input;
		persist_dot = 0;
	}
	else {
		global_button_input = prev_button_val;
	}
}

int main (void)
{
	int pattern_match = 0;
	state_t current_state = S0;
	event_t current_event;
	
	init_system();
	init_int0();
	
	while (!pattern_match)
	{
		GLCD_DisplayString(7,1,1,"STATUS: RUNNING");

		checkIfDotOrDash();
		
			if (global_button_input == 0) {
				current_event = DOT;
				GLCD_DisplayString(3,1,1,"INPUT:  DOT");
			}
			else if (global_button_input == 1) {
				current_event = DASH;
				GLCD_DisplayString(3,1,1,"INPUT: DASH");
			}
		
			if(persist_dash == MAX_PERSIST || persist_dot == MAX_PERSIST) {
				if(global_button_input == 1) {
					GLCD_DisplayString(3,1,1,"INPUT: DASH");
					persist_dash = 0;
				}
				else {
					GLCD_DisplayString(3,1,1,"INPUT:  DOT");
					persist_dot = 0;
				}
				current_state = transition_matrix[current_state][current_event];
			}
			
			sprintf(curr_state, "%02d", current_state);
			GLCD_DisplayString(1, 1, 1, (unsigned char *)curr_state);
			
			if (current_state == S7) { 
				pattern_match = 1;
				GLCD_Clear(White);
				GLCD_DisplayString(7,1,1,"STATUS: COMPLETE");
			}
	}
}
