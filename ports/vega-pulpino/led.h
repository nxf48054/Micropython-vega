#include "py/obj.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#define PIN22_IDX 22u
#define PIN23_IDX 23u
#define PIN24_IDX 24u

#define NUM_LEDS 3
#define MICROPY_HW_LED_ON(pin)      (mp_hal_pin_high(pin))
#define MICROPY_HW_LED_OFF(pin)     (mp_hal_pin_low(pin))

#define MICROPY_HW_LED0_NAME "red"
#define MICROPY_HW_LED1_NAME "green"
#define MICROPY_HW_LED2_NAME "blue"

typedef void(*PIN_INIT)(void);

typedef enum {
	PYB_LED_R = 1,
	PYB_LED_G,
	PYB_LED_B,
} pyb_led_t;

typedef struct _pyb_led_obj_t{
	mp_obj_base_t base;
	pyb_led_t led_id;
	PIN_INIT init_pin;
	pin_obj_t *led_pin;
} pyb_led_obj_t;

void led_init(void);
void led_state(pyb_led_t led, int state);
void led_toggle(pyb_led_t led);
void led_debug(int value, int delay);

extern const mp_obj_type_t pyb_led_type;
