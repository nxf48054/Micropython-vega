#include "stdint.h"
extern volatile uint32_t mTicks;
uint32_t systick_current_mills();
void systick_sleep(uint32_t ms);
