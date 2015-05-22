#include <lpc17xx.h>
#include "glcd.h"
#include <stdio.h>

uint32_t g_ms;

int timer_subroutine(uint32_t milliseconds) {
    int i, j;
		char time[64];
	
    GLCD_DisplayString(1,1,1,"START");
    for(i=0; i<milliseconds; ++i) {
			for(j=0; j<17000; ++j) {
				__nop();
      }
				
			sprintf(time, "%02d:%02d",(i/60000),(i/1000)%60);
			
			if (i % 1000 == 0) {
				 GLCD_DisplayString(7, 1, 1, (unsigned char *)time);
			}
		
    }
    GLCD_DisplayString(2,2,1,"END");
    return 0;
}


int main(void) {
    SystemInit();
    GLCD_Init();
    GLCD_Clear(White);
    __disable_irq();
    g_ms = 10000;
		timer_subroutine(g_ms);
    return 0;
}
