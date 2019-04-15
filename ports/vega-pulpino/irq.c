#include "stdint.h"

#include "py/nlr.h"
#include "py/obj.h"
#include "py/mphal.h"
#include "irq.h"

volatile uint32_t mTicks;
uint32_t g_mstatus;
bool g_disableIntCalled = false;
#ifndef SysTick_Handler
#define SysTick_Handler LPIT0_IRQHandler
#endif
void SysTick_Handler(void)
{
    SystemClearSystickFlag(); //use a fake systick, need to clear the counting interrupt flag, same as the RT
	mTicks++;
}


STATIC mp_obj_t pyb_disable_irq(void)
{
	g_disableIntCalled = true;
	return mp_obj_new_int(disable_irq());
}

MP_DEFINE_CONST_FUN_OBJ_0(pyb_disable_irq_obj, &pyb_disable_irq);

STATIC mp_obj_t pyb_enable_irq(uint n_args, const mp_obj_t* args)
{
	if(g_disableIntCalled){
		if(n_args < 1)
			enable_irq(g_mstatus);
		else
			enable_irq(mp_obj_get_int(args[0]));
		g_disableIntCalled = false;
		return mp_const_none;
	}
	nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, "Please call the disable_irq first!"));
	return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_enable_irq_obj, 0, 1 ,&pyb_enable_irq);


