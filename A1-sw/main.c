#include<lpc17xx.h>
#include"glcd.h"

/*
void BoardInit() {
    LPC_TIM0->TCR=0x02;
    LPC_TIM0>TCR=0x02;  //resettimer
    LPC_TIM0>TCR=0x01;  //enabletimer
    LPC_TIM0>MR0=2048;  //match value(canbeanything)
    LPC_TIM0>MCR|=0x03; //on match, generate interrupt and reset 
    NVIC_EnableIRQ(TIMER0_IRQn); //allow interrupts from the timer
    
    //SysTick_Config(SystemFrequency/10000);
}
*/

int timer_subroutine() {
    int i, j;
    GLCD_DisplayString(1,1,1,"START");
    for(i=0; i<60000; ++i) {
            for(j=0; j<25000; ++j) {
                __nop();
            }
    }
    GLCD_DisplayString(2,2,1,"END");
    return 0;
}


int main(void) {

    SystemInit();
    //BoardInit();
    GLCD_Init();
    GLCD_Clear(White);
    //GLCD_DisplayString(0,0,1,"Hello,world!");
    __disable_irq();
    timer_subroutine();
    return 0;
}
