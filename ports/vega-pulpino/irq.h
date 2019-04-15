#define IRQ_STATE_DISABLED (0x00000001)
#define IRQ_STATE_ENABLED  (0x00000000)

MP_DECLARE_CONST_FUN_OBJ_0(pyb_disable_irq_obj);
MP_DECLARE_CONST_FUN_OBJ_VAR_BETWEEN(pyb_enable_irq_obj);

extern uint32_t g_mstatus;
