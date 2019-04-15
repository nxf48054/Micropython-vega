/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Damien P. George
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
#include "rng.h"
#include "board.h"
#include "fsl_trng.h"
#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include <stdlib.h>
#if MICROPY_HW_ENABLE_RNG
uint32_t s_seed;
uint32_t rng_get(void) {
    uint32_t i;
    trng_config_t trngConfig;
    status_t status;
    uint32_t data;
	TRNG_GetDefaultConfig(&trngConfig);
	/* Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.
	 * It is optional.*/
	trngConfig.sampleMode = kTRNG_SampleModeVonNeumann;
	/* Initialize TRNG */
	status = TRNG_Init(TRNG, &trngConfig);
	status = TRNG_GetRandomData(TRNG, &data, 1);
	if (status == kStatus_Success)
	{
		s_seed = data;
		srand(s_seed);
	}
	return rand();
}

// Return a 30-bit hardware generated random number.
STATIC mp_obj_t pyb_rng_getnum(void) {
	return mp_obj_new_int(rng_get() >> 2);
}

MP_DEFINE_CONST_FUN_OBJ_0(pyb_rng_getnum_obj, pyb_rng_getnum);


#endif // MICROPY_HW_ENABLE_RNG
