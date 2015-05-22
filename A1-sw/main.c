#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t ms_coeff = 1000;
uint32_t sec_in_min = 60;
uint32_t ms_in_min = 60000;
uint32_t min_to_run = 10;
uint32_t magic_number = 16550;

int timer_subroutine(uint32_t milliseconds) {
    int i, j;
		char cur_time[128];
	
    for(i=1; i<=milliseconds; ++i) {
			for(j=0; j<=magic_number; ++j) {
				__nop();
      }
				
			sprintf(cur_time, "%02d:%02d",(i/ms_in_min),(i/ms_coeff)%sec_in_min);
			
			if (i % ms_coeff == 0) {
				 GLCD_DisplayString(5, 5, 1, (unsigned char *)cur_time);
			}
		
    }
    return 0;
}


int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(Black);
		GLCD_SetTextColor(Red);
		GLCD_SetBackColor(Black);
    __disable_irq();

		GLCD_DisplayString(1, 1, 1, "Status: ");
		GLCD_DisplayString(1, 9, 1, "START");
		GLCD_DisplayString(4, 5, 1, "TIMER");
	
		timer_subroutine(min_to_run * ms_in_min);
	
		GLCD_DisplayString(1, 9, 1, "END  ");
		__enable_irq();
    return 0;
}
