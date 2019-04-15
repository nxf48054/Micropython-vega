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

#include "fsl_lpspi.h"
#include "fsl_lpspi_edma.h"


#define SPI_OBJ_FLAG_ENABLED	1
#define SPI_OBJ_FLAG_BUSY		2
#define SPI_OBJ_FLAG_CPHA		3
#define SPI_OBJ_FLAG_CPOL		4


extern const mp_obj_type_t pyb_spi_type;
extern const mp_obj_type_t mp_machine_soft_spi_type;
extern const mp_obj_type_t machine_hard_spi_type;


void spi_init0(void);
void spi_init(pyb_spi_obj_t *self);
bool pyb_spi_init_c(pyb_spi_t ndx, mp_arg_val_t args[]);
void spi_transfer(pyb_spi_t ndx, size_t txLen, const uint8_t *src, size_t rxLen, uint8_t *dest, uint32_t timeout, bool isCtrlSSEL);
void spi_deinit(pyb_spi_t ndx);

int *spi_get_handle(mp_obj_t o);
