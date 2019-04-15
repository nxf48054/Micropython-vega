#pragma once
#ifndef __INCLUDED_MPCONFIGPORT_H
#define __INCLUDED_MPCONFIGPORT_H
#include <stdint.h>
#include "fsl_common.h"

#define MP_SSIZE_MAX (0x7fffffff)

#define MICRO_HW_HAS_I2C           (1)
#define MICRO_HW_HAS_LED           (1)
#define MICROPY_HW_ENABLE_RNG      (1)
#define MICROPY_HW_HAS_FS_MOUNT    (0)   
#define MICROPY_HW_HAS_SDCARD      (0)
#define MICROPY_HW_HAS_ADC         (0)
#define MICROPY_HW_HAS_FS          (0)
// options to control how MicroPython is built

// You can disable the built-in MicroPython compiler by setting the following
// config option to 0.  If you do this then you won't get a REPL prompt, but you
// will still be able to execute pre-compiled scripts, compiled with mpy-cross.
#define MICROPY_ENABLE_COMPILER     (1)
#define MICROPY_ENABLE_FINALISER    (0)

#define MICROPY_ALLOC_PATH_MAX      (256)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (16)
#define MICROPY_EMIT_X86            (0)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_COMP_MODULE_CONST   (1)
#define MICROPY_COMP_CONST          (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (1)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_RETURN_IF_EXPR (1)
#define MICROPY_MALLOC_USES_ALLOCATED_SIZE (1)
#define MICROPY_MEM_STATS           (1)
#define MICROPY_USE_READLINE_HISTORY (1)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_READER_POSIX        (0)
#define MICROPY_USE_READLINE_HISTORY (1)
#define MICROPY_GC_ALLOC_THRESHOLD  (0)
#define MICROPY_REPL_EVENT_DRIVEN   (0)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_HELPER_LEXER_UNIX   (1)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
//#define MICROPY_ENABLE_DOC_STRING   (0)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (0)
#define MICROPY_PY_ASYNC_AWAIT      (0)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FILTER  (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_MIN_MAX (1)
#define MICROPY_PY_BUILTINS_HELP            (1)
#define MICROPY_PY_BUILTINS_HELP_MODULES    (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY___FILE__         (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_MATH             (1)
#define MICROPY_PY_CMATH            (1)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_SYS_MAXSIZE      (1)
#define MICROPY_PY_SYS_STDIO_BUFFER (1)
#define MICROPY_PY_MACHINE          (1)
#define MICROPY_CPYTHON_COMPAT      (0)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_DOUBLE)

extern const struct _mp_obj_module_t machine_module;
extern const struct _mp_obj_module_t pyb_module;
extern const struct _mp_obj_module_t time_module;
extern const struct _mp_obj_module_t mcu_module;

#define MICROPY_PORT_BUILTIN_MODULES \
	{ MP_ROM_QSTR(MP_QSTR_umachine), MP_ROM_PTR(&machine_module) },	\
	{ MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&machine_module) },	\
    { MP_ROM_QSTR(MP_QSTR_pyb), MP_ROM_PTR(&pyb_module) }, \
	{ MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&time_module) }, \
	{ MP_ROM_QSTR(MP_QSTR_mcu), MP_ROM_PTR(&mcu_module) },

#define MICROPY_PORT_CONSTANTS \
	{ MP_ROM_QSTR(MP_QSTR_umachine), MP_ROM_PTR(&machine_module) },	\
	{ MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&machine_module) },	\
    { MP_ROM_QSTR(MP_QSTR_pyb), MP_ROM_PTR(&pyb_module) }, \
	{ MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&time_module) }, \
	{ MP_ROM_QSTR(MP_QSTR_mcu), MP_ROM_PTR(&mcu_module) },

// type definitions for the specific machine

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

// This port is intended to be 32-bit, but unfortunately, int32_t for
// different targets may be defined in different ways - either as int
// or as long. This requires different printf formatting specifiers
// to print such value. So, we avoid int32_t and use int directly.
#define UINT_FMT "%u"
#define INT_FMT "%d"
typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size

typedef long mp_off_t;

#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#define MICROPY_HW_BOARD_NAME "Vega Board"
#define MICROPY_HW_MCU_NAME "RISCV (RV32IM)"

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8];

extern uint32_t g_mstatus;
static inline void enable_irq(mp_uint_t state){
	__ASM volatile ("csrw mstatus, %0" : : "r"(state));
}

static inline mp_uint_t disable_irq(void){
    __ASM volatile ("csrrci %0, mstatus, 8" : "=r"(g_mstatus));

    return g_mstatus;
}
#endif
