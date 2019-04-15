#include "fsl_intmux.h"
#include "fsl_common.h"
#include "fsl_gpio.h"

NORETURN void mp_hal_raise(status_t status);

typedef struct pin_obj_t{
	PORT_Type *port;
	GPIO_Type *gpio;
	uint32_t pin;
} pin_obj_t;

#define mp_hal_pin_obj_t const pin_obj_t*

static inline void mp_hal_pin_write(GPIO_Type *pPort, uint32_t pin, uint32_t bitLevel) {
	GPIO_WritePinOutput(pPort, pin, bitLevel);
}
static inline void mp_hal_pin_low(const pin_obj_t *pPin){
	GPIO_WritePinOutput(pPin->gpio, pPin->pin, 0);
}
static inline void mp_hal_pin_high(const pin_obj_t *pPin) {
	GPIO_WritePinOutput(pPin->gpio, pPin->pin, 1);
	
}
static inline void mp_hal_pin_toggle(const pin_obj_t *pPin){
	uint32_t a, pinNdx = pPin->pin;
	a = (0 == GPIO_ReadPinInput(pPin->gpio, pinNdx) );
	GPIO_WritePinOutput(pPin->gpio, pinNdx, a);
	
}
