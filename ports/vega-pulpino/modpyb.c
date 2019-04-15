/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdint.h>
#include <stdio.h>
#include "mpconfigport.h"
#include "fsl_common.h"

#include "py/mpstate.h"
#include "py/nlr.h"
#include "py/obj.h"
#include "py/gc.h"
#include "py/builtin.h"
#include "py/mphal.h"
#include "lib/utils/pyexec.h"
#include "extmod/utime_mphal.h"
#include "i2c.h"
#include "led.h"
#include "rng.h"
#include "irq.h"
#include "test.h"
#include "modmachine.h"


STATIC const mp_rom_map_elem_t pyb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_pyb) },
#if MICROPY_HW_HAS_FS_MOUNT
    { MP_ROM_QSTR(MP_QSTR_mount), MP_ROM_PTR(&mp_vfs_mount_obj) },
#endif
#if MICROPY_HW_HAS_SDCARD
    { MP_ROM_QSTR(MP_QSTR_SD), MP_ROM_PTR(&pyb_sdcard_obj) }, // now obsolete
    { MP_ROM_QSTR(MP_QSTR_SDCard), MP_ROM_PTR(&pyb_sdcard_type) },
#endif
#if MICROPY_HW_HAS_ADC
    { MP_ROM_QSTR(MP_QSTR_ADC), MP_ROM_PTR(&pyb_adc_type) },
    { MP_ROM_QSTR(MP_QSTR_ADCAll), MP_ROM_PTR(&pyb_adc_all_type) },
#endif
#if MICROPY_HW_HAS_FS
    { MP_ROM_QSTR(MP_QSTR_VfsFat), MP_ROM_PTR(&mp_fat_vfs_type) },
#endif
#if MICRO_HW_HAS_I2C
    { MP_ROM_QSTR(MP_QSTR_I2C), MP_ROM_PTR(&pyb_i2c_type) },
#endif
#if MICRO_HW_HAS_LED
    { MP_ROM_QSTR(MP_QSTR_LED), MP_ROM_PTR(&pyb_led_type) },
#endif
    { MP_ROM_QSTR(MP_QSTR_pyboard), MP_ROM_PTR(&pyb_pyboard_type) },
#if MICROPY_HW_ENABLE_RNG
	{ MP_ROM_QSTR(MP_QSTR_rng), MP_ROM_PTR(&pyb_rng_getnum_obj) },
#endif
	{ MP_ROM_QSTR(MP_QSTR_hard_reset),       MP_ROM_PTR(&machine_reset_obj) },
	{ MP_ROM_QSTR(MP_QSTR_info),       MP_ROM_PTR(&machine_info_obj) },
 	{ MP_ROM_QSTR(MP_QSTR_unique_id),  MP_ROM_PTR(&machine_unique_id_obj) },
 	{ MP_ROM_QSTR(MP_QSTR_freq),       MP_ROM_PTR(&machine_freq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_disable_irq), MP_ROM_PTR(&pyb_disable_irq_obj) },
	{ MP_ROM_QSTR(MP_QSTR_enable_irq), MP_ROM_PTR(&pyb_enable_irq_obj) },
}; 
  
STATIC MP_DEFINE_CONST_DICT(pyb_module_globals, pyb_module_globals_table);

const mp_obj_module_t pyb_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&pyb_module_globals,
};   

