#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

#define FIVE_SEC 25000 * 1000 * 5
#define TEN_SEC  25000 * 1000 * 10
#define LED_PIN 28

uint32_t g_ms;
uint32_t ms_coeff = 1000;
uint32_t sec_in_min = 60;
uint32_t ms_in_min = 60000;
uint32_t min_to_run = 10;

char curr_count[16];
volatile int input;
volatile uint32_t global_time;
volatile char counter[64];
int int0_val;
volatile int tim0_val;

volatile int button_press_count = 0;
int led_delay = 0;

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
	//LPC_GPIOINT->IO2IntEnR |= 1 << 10; // rising edge of P2.10	BUTTON IS RELEASED
	NVIC_EnableIRQ(EINT3_IRQn);
	
	// Timer to handle debouncing
	LPC_TIM1->TCR = 0x02;	// reset timer
	LPC_TIM1->TCR = 0x01; // enable timer
	LPC_TIM1->MR0 = FIVE_SEC; // match register
	LPC_TIM1->MCR |= 0x03; // on match, generate interrupt and reset
	NVIC_EnableIRQ(TIMER1_IRQn); // enable timer interrupts
	
	// LED init
	LPC_GPIO1->FIODIR |= 0xB0000000;	// LEDs on Port 1	
	LPC_GPIO2->FIODIR |= 0x0000007C;	// LEDs on Port 2
	
}

void TIMER1_IRQHandler() {
		LPC_TIM1->IR |= 0x01;	// clear the interrupt.
		NVIC_ClearPendingIRQ(EINT3_IRQn);
		NVIC_EnableIRQ(EINT3_IRQn);		// Re-enable the button interrupt.
}


void EINT3_IRQHandler()
{
	// Clear the interrupt on EINT0
	LPC_GPIOINT->IO2IntClr |= 1 << 10;
		
	// Turn on the LED
	LPC_GPIO1->FIOSET = 1 << LED_PIN;

	button_press_count = button_press_count + 1;
	
	int0_val =~ (LPC_GPIO2->FIOPIN >> 10) & 0x01;
	
	NVIC_DisableIRQ(EINT3_IRQn);
}

int main (void)
{
	init_system();
	init_intr();

	
	GLCD_DisplayString(1, 1, 1, "COUNT: ");
	sprintf(curr_count, "%02d", 0);
	GLCD_DisplayString(1, 8, 1, (unsigned char *)curr_count);
	
	while (1)
	{
			GLCD_DisplayString(7,1,1,"STATUS: RUNNING");
			sprintf(curr_count, "%02d", button_press_count);
			GLCD_DisplayString(1, 8, 1, (unsigned char *)curr_count);
		
			for(led_delay = 0; led_delay < 100; ++led_delay) {}
			
			// Turn off the LED
			LPC_GPIO1->FIOSET = 0 << 28;
	}
}
