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

static char *stack_top;
static char heap[100*1024];
int main(int argc, char **argv) {
    int stack_dummy;
    // Initialization block taken from led_fade.c
    {
        BOARD_InitPins();
        BOARD_BootClockRUN();
        BOARD_InitDebugConsole();
    }
    
    //note: the value from *.ld is the value store in the address of the ram, such as _estack is the value store in the real address in ram, so should use & to get the real address
    stack_top = (char*)&stack_dummy;//(char*)&_estack;
    mp_stack_set_top((void*) stack_top);
    mp_stack_set_limit(2048);
    gc_init(heap, heap + sizeof(heap)); 
    //gc_init(&_heap_start, &_heap_end);
    mp_init();
    pyexec_friendly_repl();
    mp_deinit();
    return 0;
}

void gc_collect(void) {
	printf("gc_collect\n");
    void *dummy ;//= (void*)(&_estack - &_heap_end - 1024);
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)&stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info();
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

