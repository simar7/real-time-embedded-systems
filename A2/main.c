#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

char curr_state[16];
volatile int input;
volatile uint32_t global_time;
volatile char counter[64];
int int0_val;
volatile int tim0_val;

volatile int global_button_input = 0;
unsigned char Dot_probability;
unsigned char Dash_probability;

#define TOTAL_STATES 8
#define TOTAL_EVENTS 2
#define NOISE_HANDLER 3
#define TS 9999999

typedef enum { S0, S1, S2, S3, S4, S5, S6, S7 } state_t;
typedef enum { DOT, DASH } event_t;
	
state_t transition_matrix[TOTAL_STATES][TOTAL_EVENTS] = {
	{S1, S0},	// S0
	{S1, S2}, // S1
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
	
	// Timer to handle debouncing
	LPC_TIM1->TCR = 0x02;	// reset timer
	LPC_TIM1->TCR = 0x01; // enable timer
	LPC_TIM1->MR0 = 1024; // match register
	LPC_TIM1->MCR |= 0x03; // on match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER1_IRQn); // enable timer interrupts
	
}

void TIMER1_IRQHandler() {
		static unsigned char noise_counter = 0;
		int button_input =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01;
	
		LPC_TIM1->IR |= 0x01;	// clear the interrupt.
	
		if(button_input == 1) {
				if(Dot_probability != 0) {
					Dot_probability--;
				}
		}
		else {
			if(Dot_probability != 0xff) {
					Dot_probability++;
			}
		}

		if(++noise_counter >= NOISE_HANDLER) {
			noise_counter = 0;
			if(Dot_probability == 0xff) {
				if(Dash_probability != 0xff)
					Dash_probability++;
			}
			else if(Dot_probability == 0x00)
				Dash_probability = 0x00;
		}	
}


void EINT3_IRQHandler()
{
	// Clear the interrupt on EINT0
	LPC_GPIOINT->IO2IntClr |= 1 << 10;
	
	int0_val =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01;

	if (int0_val == 1)		// Button is pressed
	{
		// Start Timer
		global_time = 0;
		LPC_TIM0->TCR = 0x01;
		LPC_TIM0->PR  = 0x00; 
	}
	else if (int0_val == 0)		// Button is released
	{
		// Stop Timer
		tim0_val = LPC_TIM0->TC;
		global_time = tim0_val;
		LPC_TIM0->TCR = 0x02;
		LPC_TIM0->PR  = 0x00; 
	}
}

int main (void)
{
	int pattern_match = 0;
	state_t current_state = S0;
	event_t current_event;
	
	init_system();
	init_intr();
	
	GLCD_DisplayString(3,1,1,"INPUT:    ");
	GLCD_DisplayString(1, 1, 1, "STATE: ");
	sprintf(curr_state, "%02d", current_state);
	GLCD_DisplayString(1, 8, 1, (unsigned char *)curr_state);
	
	while (!pattern_match)
	{
			GLCD_DisplayString(7,1,1,"STATUS: RUNNING");
		
			if (Dot_probability == 0xff && global_time < TS && global_time > 0) {
				current_event = DOT;
				GLCD_DisplayString(3,1,1,"INPUT:  DOT");
	
				current_state = transition_matrix[current_state][current_event];
				sprintf(curr_state, "%02d", current_state);
				GLCD_DisplayString(1, 8, 1, (unsigned char *)curr_state);
				
				global_time = 0;
			}
			else if (Dash_probability == 0xff && global_time >= TS && global_time > 0) {
				current_event = DASH;
				GLCD_DisplayString(3,1,1,"INPUT: DASH");

				current_state = transition_matrix[current_state][current_event];
				sprintf(curr_state, "%02d", current_state);
				GLCD_DisplayString(1, 8, 1, (unsigned char *)curr_state);
				
				global_time = 0;
			}
			
			if (current_state == S7) { 
				pattern_match = 1;
				GLCD_DisplayString(7,1,1,"STATUS: CORRECT!");
			}
	}
}
