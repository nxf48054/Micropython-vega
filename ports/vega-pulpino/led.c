#include <stdio.h>
  
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "led.h"
#include "systick.h"

/* Define the init structure for the output LED pin*/
gpio_pin_config_t led_config = {
	kGPIO_DigitalOutput, 0,
};
//use function pointer to do the gpio init:
void Init_LED_PIN_R(void)
{
	PORT_SetPinMux(PORTA, PIN24_IDX, kPORT_MuxAsGpio);
    GPIO_PinInit(GPIOA, PIN24_IDX, &led_config);
}  

void Init_LED_PIN_G(void)
{
	PORT_SetPinMux(PORTA, PIN23_IDX, kPORT_MuxAsGpio);
	GPIO_PinInit(GPIOA, PIN23_IDX, &led_config);
} 

void Init_LED_PIN_B(void)
{
	PORT_SetPinMux(PORTA, PIN22_IDX, kPORT_MuxAsGpio);
	GPIO_PinInit(GPIOA, PIN22_IDX, &led_config);
}
//use pin-struct to do the gpio init:
void Init_Led_Pin(pin_obj_t *pin)
{
	PORT_SetPinMux(pin->port, pin->pin, kPORT_MuxAsGpio);
    GPIO_PinInit(pin->gpio, pin->pin, &led_config);
}  

pin_obj_t pyb_pin_obj[3] = {
	{.port = PORTA, .gpio = GPIOA, .pin = PIN24_IDX}, 
	{.port = PORTA, .gpio = GPIOA, .pin = PIN23_IDX}, 
	{.port = PORTA, .gpio = GPIOA, .pin = PIN22_IDX},
};

pyb_led_obj_t pyb_led_obj[3] = {
	{.base = &pyb_led_type, .led_id = PYB_LED_R, .led_pin = &pyb_pin_obj[0]},
	{.base = &pyb_led_type, .led_id = PYB_LED_G, .led_pin = &pyb_pin_obj[1]},
	{.base = &pyb_led_type, .led_id = PYB_LED_B, .led_pin = &pyb_pin_obj[2]},
};

void led_init0()
{
	#if !defined(MICROPY_HW_LED0_NAME)
	pyb_led_obj[0].init_pin = NULL;
	#else
	pyb_led_obj[0].init_pin = Init_LED_PIN_R;
	#endif

	#if !defined(MICROPY_HW_LED1_NAME)
	pyb_led_obj[1].init_pin = NULL;
	#else
	pyb_led_obj[1].init_pin = Init_LED_PIN_G;
	#endif

	#if !defined(MICROPY_HW_LED2_NAME)
	pyb_led_obj[2].init_pin = NULL;
	#else
	pyb_led_obj[2].init_pin = Init_LED_PIN_B;
	#endif	
}

void led_state(pyb_led_t led, int state) {
    if (led < 1 || led > NUM_LEDS) {
        return;
    } 
	const pin_obj_t *led_pin = pyb_led_obj[led-1].led_pin;
    if (state == 0) {
        // turn LED off
        MICROPY_HW_LED_OFF(led_pin);
    } else {
        // turn LED on
        MICROPY_HW_LED_ON(led_pin);
    }
}
  
void led_toggle(pyb_led_t led) {
    if (led < 1 || led > NUM_LEDS) {
        return;
    }
    const pin_obj_t *led_pin = pyb_led_obj[led - 1].led_pin;
	mp_hal_pin_toggle(led_pin);
}

void led_obj_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_led_obj_t *self = self_in;
    mp_printf(print, "LED(%lu)", self->led_id);
}

STATIC mp_obj_t led_obj_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // get led number
    mp_int_t led_id = mp_obj_get_int(args[0]);

    // check led number
    if (!(1 <= led_id && led_id <= NUM_LEDS)) {
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "LED(%d) does not exist", led_id));
    }
#if INIT_LED_PIN_WITH_FUNC_PTR
	//init the specify pin with function pointer
	pyb_led_obj[led_id - 1].init_pin();
#else
	//init by the init_function with a pin_table
	Init_Led_Pin(pyb_led_obj[led_id - 1].led_pin);
#endif
    // return static led object
    return (mp_obj_t)&pyb_led_obj[led_id - 1];
}

/// \method on()
/// Turn the LED on.
mp_obj_t led_obj_on(mp_obj_t self_in) {
    pyb_led_obj_t *self = self_in;
    led_state(self->led_id, 1);
    return mp_const_none;
}

/// \method off()
/// Turn the LED off.
mp_obj_t led_obj_off(mp_obj_t self_in) {
    pyb_led_obj_t *self = self_in;
    led_state(self->led_id, 0);
    return mp_const_none;
}

/// \method toggle()
/// Toggle the LED between on and off.
mp_obj_t led_obj_toggle(mp_obj_t self_in) {
    pyb_led_obj_t *self = self_in;
    led_toggle(self->led_id);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_on_obj, led_obj_on);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_off_obj, led_obj_off);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(led_obj_toggle_obj, led_obj_toggle);
  
STATIC const mp_rom_map_elem_t led_locals_dict_table[] = { 
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&led_obj_on_obj) },
	{ MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&led_obj_off_obj) },
	{ MP_ROM_QSTR(MP_QSTR_toggle), MP_ROM_PTR(&led_obj_toggle_obj) }, 
	//const value for user to easy use 
	{ MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(PYB_LED_R)},
	{ MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(PYB_LED_G)},
	{ MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(PYB_LED_B)},
	
};
  
STATIC MP_DEFINE_CONST_DICT(led_locals_dict, led_locals_dict_table);
const mp_obj_type_t pyb_led_type = {
    { &mp_type_type },
	.name = MP_QSTR_LED,
	.print = led_obj_print,
	.make_new = led_obj_make_new,
	.locals_dict = (mp_obj_dict_t*)&led_locals_dict,
};
