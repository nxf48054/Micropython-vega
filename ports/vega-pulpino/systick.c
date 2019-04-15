#include "systick.h"
extern volatile uint32_t mTicks;
static inline uint32_t HAL_GetTicks()
{
	return mTicks;
}

uint32_t systick_current_mills()
{
	return HAL_GetTicks();
}
void systick_sleep(uint32_t ms)
{
	volatile uint32_t curr_ticks = systick_current_mills();
	while((systick_current_mills() - curr_ticks) < ms);
}
