#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_ms;

int timer_subroutine(uint32_t milliseconds) {
    int i, j;
		char time[128];
	
    GLCD_DisplayString(1,1,1,"A1 - START");
		GLCD_DisplayString(2,1,1,"Author: s244sing");
    for(i=1; i<=milliseconds; ++i) {
			for(j=0; j<=16550; ++j) {
				__nop();
      }
				
			sprintf(time, "%02d:%02d",(i/60000),(i/1000)%60);
			
			if (i % 1000 == 0) {
				 GLCD_DisplayString(7, 1, 1, (unsigned char *)time);
			}
		
    }
    GLCD_DisplayString(3,1,1,"A1 - END");
    return 0;
}


int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    __disable_irq();
    g_ms = 6000000;
		timer_subroutine(g_ms);
		__enable_irq();
    return 0;
}
