#include "stdint.h"
volatile uint32_t mTicks;
#ifndef SysTick_Handler
#define SysTick_Handler LPIT0_IRQHandler
void SysTick_Handler(void)
{
    SystemClearSystickFlag(); //use a fake systick, need to clear the counting interrupt flag, same as the RT
	mTicks++;
}
#endif
