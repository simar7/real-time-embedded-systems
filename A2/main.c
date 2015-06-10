#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_ms = 0;
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
unsigned char Key_Hit;
unsigned char Long_Key_Hit;

#define MAX_PERSIST 3
#define TOTAL_STATES 8
#define TOTAL_EVENTS 2
// #define BUTTON int0_val =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01
volatile unsigned char Div18Counter = 0;

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

void init_intr()
{
	// Set INT0 as input
	LPC_PINCON->PINSEL4 &= ~(3<<20);		// P2.10 is GPIO
	LPC_GPIO2->FIODIR		&= ~(1<<1);		// P2.10 is input
	
	LPC_GPIOINT->IO2IntEnF |= 1 << 10; // falling edge of P2.10		BUTTON IS PRESSED
	LPC_GPIOINT->IO2IntEnR |= 1 << 10; // rising edge of P2.10	BUTTON IS RELEASED
	NVIC_EnableIRQ(EINT3_IRQn);
	
	LPC_TIM1->TCR = 0x02;	// reset timer
	LPC_TIM1->TCR = 0x01; // enable timer
	LPC_TIM1->MR0 = 4096; // match register TODO: Check what value works.
	LPC_TIM1->MCR |= 0x03; // on match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER1_IRQn); // enable timer interrupts
	
}

void TIMER1_IRQHandler() {
		LPC_TIM1->IR |= 0x01;	// clear the interrupt.

		if(global_button_input == 0) {
				if(Key_Hit != 0) {
					Key_Hit--;
				}
		}
		else {
			if(Key_Hit != 0xff) {
					Key_Hit++;
			}
		}

		if(++Div18Counter >= 18) {
			Div18Counter = 0;
			if(Key_Hit == 0xff) {
				if(Long_Key_Hit != 0xff)
					Long_Key_Hit++;
			}
			else if(Key_Hit == 0x00)
				Long_Key_Hit = 0x00;
		}
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
		LPC_TIM0->MR0 = 25000;
		LPC_TIM0->PR  = 0x00; 
	}
	else if (int0_val == 1)		// Button is released
	{
		tim0_val = LPC_TIM0->TC;
		// Stop Timer
		LPC_TIM0->TCR = 0x02;
	}

	global_time = tim0_val;
	global_button_input = int0_val;
	
}

void checkIfDotOrDash() {
	if (persist_dash == MAX_PERSIST) {		// DASH
		global_button_input = 1;
		prev_button_val = global_button_input;
		persist_dash = 0;
	} else if(persist_dot == MAX_PERSIST) {			// DOT
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
	init_intr();
	
	while (!pattern_match)
	{
		GLCD_DisplayString(7,1,1,"STATUS: RUNNING");

		//checkIfDotOrDash();
		
			if (Key_Hit == 0xff) {
				current_event = DOT;
				GLCD_DisplayString(3,1,1,"INPUT:  DOT");
				current_state = transition_matrix[current_state][current_event];
				sprintf(curr_state, "%02d", current_state);
				GLCD_DisplayString(1, 1, 1, (unsigned char *)curr_state);
			}
			else if (Key_Hit == 0x00) {
				// Release
			}
			else if (Long_Key_Hit == 0xff) {
				current_event = DASH;
				GLCD_DisplayString(3,1,1,"INPUT: DASH");
				current_state = transition_matrix[current_state][current_event];
				sprintf(curr_state, "%02d", current_state);
				GLCD_DisplayString(1, 1, 1, (unsigned char *)curr_state);
			}
		
			
			if (current_state == S7) { 
				pattern_match = 1;
				GLCD_Clear(White);
				GLCD_DisplayString(7,1,1,"STATUS: COMPLETE");
			}
	}
}
