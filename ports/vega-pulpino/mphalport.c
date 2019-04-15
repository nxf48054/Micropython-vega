#include <string.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"

extern __IO uint32_t mTicks;
const byte mp_hal_status_to_errno_table[2] = {
    [kStatus_Success] = 0,
    [kStatus_Fail] = MP_EIO,
};
mp_uint_t mp_hal_ticks_ms(void) { return mTicks; }
void mp_hal_set_interrupt_char(char c) {}
NORETURN void mp_hal_raise(status_t status) {
    mp_raise_OSError(mp_hal_status_to_errno_table[status]);
}
