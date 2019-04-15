#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/stackctrl.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/pyexec.h"

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "gccollect.h"
#include "pin_mux.h"
#include "clock_config.h"

#define INIT_LED_PIN_WITH_FUNC_PTR (0)

void Systick_Init(void)
{   
    CLOCK_SetIpSrc(kCLOCK_Lpit0, kCLOCK_IpSrcFircAsync);//stupid error: miss this line, the lpit0 without a src-clock
	SystemSetupSystick(1000,7);
	SystemClearSystickFlag();
}

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    int32_t c = 0;
    while (c<=0) {
        c = GETCHAR();
    }
    return c;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    while (len--) {
        PUTCHAR(*str++);
    }
}
   
void do_str(const char *src, mp_parse_input_kind_t input_kind) {
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_lexer_t *lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, src, strlen(src), 0);
        qstr source_name = lex->source_name;
        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);
        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, MP_EMIT_OPT_NONE, true);
        mp_call_function_0(module_fun);
        nlr_pop();
    } else {
        // uncaught exception
        mp_obj_print_exception(&mp_plat_print, (mp_obj_t)nlr.ret_val);
    }
}

extern uint32_t __stack;
extern uint32_t _stack_size;
extern uint32_t _estack;
extern uint32_t _heap_start;
extern uint32_t _heap_end;
int main(int argc, char **argv) {
    int stack_dummy;
    // Initialization block taken from led_fade.c
    {
        BOARD_InitPins();
        BOARD_BootClockRUN();
        BOARD_InitDebugConsole();
		Systick_Init(); // init the systick timer
    }
    
    //note: the value from *.ld is the value store in the address of the ram, such as _estack is the value store in the real address in ram, so should use & to get the real address
#if INIT_LED_PIN_WITH_FUNC_PTR
	led_init0();
#endif
    mp_stack_set_top(&_estack);
    mp_stack_set_limit(&_stack_size);
    gc_init(&_heap_start, &_heap_end);
    mp_init();
    pyexec_friendly_repl();
    mp_deinit();
    return 0;
}

void gc_collect(void) {
#if ENABLE_GC_OUTPUT
    printf("gc_collect\n");
#endif
    gc_collect_start();
    gc_collect_root((void**)&__stack, ((uint32_t)&_estack - (uint32_t)&__stack) / sizeof(uint32_t) );
    gc_collect_end();
#if ENABLE_GC_OUTPUT
    gc_dump_info();
#endif

}

mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);

void nlr_jump_fail(void *val) {
    while (1);
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

