#include <stdio.h>
  
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
  
mp_obj_t pyboard_display() {

    printf("Hello PYBoard With VEGA!\n");
    return mp_const_none;
}
  
STATIC MP_DEFINE_CONST_FUN_OBJ_0(pyboard_display_obj, pyboard_display);
  
STATIC const mp_rom_map_elem_t pyboard_locals_dict_table[] = {
  
    { MP_ROM_QSTR(MP_QSTR_display), MP_ROM_PTR(&pyboard_display_obj) },
  
};
  
STATIC MP_DEFINE_CONST_DICT(pyboard_locals_dict, pyboard_locals_dict_table);
const mp_obj_type_t pyb_pyboard_type = {
    { &mp_type_type },
    .name = MP_QSTR_pyboard,
    .locals_dict = (mp_obj_dict_t*)&pyboard_locals_dict,
};
