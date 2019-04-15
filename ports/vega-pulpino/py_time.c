#include <stdio.h>
  
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "systick.h"

static mp_obj_t py_time_ticks()
{
    return mp_obj_new_int(systick_current_mills());
}

static mp_obj_t py_time_sleep(mp_obj_t ms)
{
    systick_sleep(mp_obj_get_int(ms));
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(py_time_ticks_obj, py_time_ticks);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_time_sleep_obj, py_time_sleep);
  
STATIC const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_time) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_ticks),   (mp_obj_t)&py_time_ticks_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sleep),   (mp_obj_t)&py_time_sleep_obj },
};
  
STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

const mp_obj_module_t time_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_t)&globals_dict,
};
