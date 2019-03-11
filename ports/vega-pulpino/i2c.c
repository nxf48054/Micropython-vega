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
#define _I2C_C_
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_intmux.h"
#include "fsl_lpi2c.h"
#include "pin_mux.h"
#include "i2c.h"



/// \moduleref pyb
/// \class I2C - a two-wire serial protocol
///
/// I2C is a two-wire protocol for communicating between devices.  At the physical
/// level it consists of 2 wires: SCL and SDA, the clock and data lines respectively.
///
/// I2C objects are created attached to a specific bus.  They can be initialised
/// when created, or initialised later on:
///
///     from pyb import I2C
///
///     i2c = I2C(1)                         # create on bus 1
///     i2c = I2C(1, I2C.MASTER)             # create and init as a master
///     i2c.init(I2C.MASTER, baudrate=20000) # init as a master
///     i2c.init(I2C.SLAVE, addr=0x42)       # init as a slave with given address
///     i2c.deinit()                         # turn off the peripheral
///
/// Printing the i2c object gives you information about its configuration.
///
/// Basic methods for slave are send and recv:
///
///     i2c.send('abc')      # send 3 bytes
///     i2c.send(0x42)       # send a single byte, given by the number
///     data = i2c.recv(3)   # receive 3 bytes
///
/// To receive inplace, first create a bytearray:
///
///     data = bytearray(3)  # create a buffer
///     i2c.recv(data)       # receive 3 bytes, writing them into data
///
/// You can specify a timeout (in ms):
///
///     i2c.send(b'123', timeout=2000)   # timout after 2 seconds
///
/// A master must specify the recipient's address:
///
///     i2c.init(I2C.MASTER)
///     i2c.send('123', 0x42)        # send 3 bytes to slave with address 0x42
///     i2c.send(b'456', addr=0x42)  # keyword for address
///
/// Master also has other methods:
///
///     i2c.is_ready(0x42)           # check if slave 0x42 is ready
///     i2c.scan()                   # scan for slaves on the bus, returning
///                                  #   a list of valid addresses
///     i2c.mem_read(3, 0x42, 2)     # read 3 bytes from memory of slave 0x42,
///                                  #   starting at address 2 in the slave
///     i2c.mem_write('abc', 0x42, 2, timeout=1000)
#define PYB_I2C_MASTER (0)
#define PYB_I2C_SLAVE  (1)
#define IS_MST_IDLE(self)  ((self->pI2C->MSR & 1<<25) == 0)

#define I2C_RELEASE_SDA_PORT PORTE
#define I2C_RELEASE_SCL_PORT PORTE
#define I2C_RELEASE_SDA_GPIO GPIOE
#define I2C_RELEASE_SDA_PIN 29U
#define I2C_RELEASE_SCL_GPIO GPIOE
#define I2C_RELEASE_SCL_PIN 30U
#define I2C_RELEASE_BUS_COUNT 100U

void BOARD_I2C_ReleaseBus(void);
pyb_i2c_obj_t pyb_i2c_obj[4] = {
    {{&pyb_i2c_type}, LPI2C0, PYB_I2C_0, kCLOCK_Lpi2c0, kCLOCK_IpSrcFircAsync, LPI2C0_IRQn, 100000, 0, 0, 1, {0}},
    {{&pyb_i2c_type}, LPI2C1, PYB_I2C_1, kCLOCK_Lpi2c1, kCLOCK_IpSrcFircAsync, LPI2C1_IRQn, 100000, 0, 0, 1, {0}},
    {{&pyb_i2c_type}, LPI2C2, PYB_I2C_2, kCLOCK_Lpi2c2, kCLOCK_IpSrcFircAsync, LPI2C2_IRQn, 100000, 0, 0, 1, {0}},
    {{&pyb_i2c_type}, LPI2C3, PYB_I2C_3, kCLOCK_Lpi2c3, kCLOCK_IpSrcFircAsync, LPI2C3_IRQn, 100000, 0, 0, 1, {0}},
};

#define NUM_BAUDRATE_TIMINGS MP_ARRAY_SIZE(pyb_i2c_baudrate_timing)

STATIC void i2c_set_baudrate(uint32_t ndx, uint32_t baudrate) {
    if (baudrate <= 1000 * 1000) {
		pyb_i2c_obj[ndx].mstCfg.baudRate_Hz = baudrate;
		pyb_i2c_obj[ndx].mstBaudrate = baudrate;
	} else {
	    nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
	                                            "Unsupported I2C baudrate: %lu", baudrate));
	}
}

uint32_t i2c_get_baudrate(uint32_t ndx) {
    return pyb_i2c_obj[ndx].mstCfg.baudRate_Hz;
}


void i2c_init0(void) {
	#if !defined(MICROPY_HW_I2C0_NAME)
	pyb_i2c_obj[0].pI2C = 0;
	#endif

	#if !defined(MICROPY_HW_I2C1_NAME)
	pyb_i2c_obj[1].pI2C = 0;
	#endif

	#if !defined(MICROPY_HW_I2C2_NAME)
	pyb_i2c_obj[2].pI2C = 0;
	#endif

	#if !defined(MICROPY_HW_I2C3_NAME)
	pyb_i2c_obj[3].pI2C = 0;
	#endif

}

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}
void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortE);
    PORT_SetPinConfig(I2C_RELEASE_SCL_PORT, I2C_RELEASE_SCL_PIN, &i2c_pin_config);
    PORT_SetPinConfig(I2C_RELEASE_SDA_PORT, I2C_RELEASE_SDA_PIN, &i2c_pin_config);

    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}
volatile bool completionFlag = false;
volatile bool nakFlag = false;
lpi2c_master_handle_t g_m_handle;

static void lpi2c_master_callback(LPI2C_Type *base, lpi2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        completionFlag = true;
    }
    /* Signal transfer success when received success status. */
    if (status == kStatus_LPI2C_Nak)
    {
        nakFlag = true;
    }
}
static bool LPI2C_ReadBlock(
    LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize)
{
    lpi2c_master_transfer_t masterXfer;
    status_t reVal = kStatus_Fail;

    memset(&masterXfer, 0, sizeof(masterXfer));
    masterXfer.slaveAddress = device_addr;
    masterXfer.direction = kLPI2C_Read;
    masterXfer.subaddress = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data = rxBuff;
    masterXfer.dataSize = rxSize;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    reVal = LPI2C_MasterTransferNonBlocking(base, &g_m_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return false;
    }
    /*  wait for transfer completed. */
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;

    if (completionFlag == true)
    {
        completionFlag = false;
        return reVal;
    }

}
static bool LPI2C_WriteBlock(LPI2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t*buf, uint32_t len)
{
    lpi2c_master_transfer_t masterXfer;
    status_t reVal = kStatus_Fail;

    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = device_addr;
    masterXfer.direction = kLPI2C_Write;
    masterXfer.subaddress = reg_addr;
    masterXfer.subaddressSize = 1;
    masterXfer.data = buf;
    masterXfer.dataSize = len;
    masterXfer.flags = kLPI2C_TransferDefaultFlag;

    /*  direction=write : start+device_write;cmdbuff;xBuff; */
    /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */

    reVal = LPI2C_MasterTransferNonBlocking(base, &g_m_handle, &masterXfer);
    if (reVal != kStatus_Success)
    {
        return false;
    }

    /*  wait for transfer completed. */
    while ((!nakFlag) && (!completionFlag))
    {
    }

    nakFlag = false;

    if (completionFlag == true)
    {
        completionFlag = false;
        return reVal;
    }
    else
    {
        return kStatus_Fail;
    }
}

bool i2c_init(uint32_t ndx, uint32_t baudRate) {
	
    CLOCK_EnableClock(kCLOCK_Rgpio1);
	BOARD_I2C_ReleaseBus();
	BOARD_I2C_ConfigurePins();

	pyb_i2c_obj_t *pOb = &pyb_i2c_obj[ndx];  
    CLOCK_SetIpSrc(pOb->myClock, pOb->myClockSrc);  
	INTMUX_Init(INTMUX0);
    INTMUX_EnableInterrupt(INTMUX0, 0, pOb->irqn);
	LPI2C_MasterGetDefaultConfig(&pOb->mstCfg);
	uint32_t sourceClock = CLOCK_GetIpFreq(pOb->myClock);
	pOb->mstCfg.baudRate_Hz = baudRate;
	LPI2C_MasterInit(pOb->pI2C, &pOb->mstCfg, sourceClock);
	LPI2C_MasterTransferCreateHandle(pOb->pI2C, &g_m_handle, lpi2c_master_callback, NULL);
	return true;
}

void i2c_deinit(uint32_t ndx) {
	pyb_i2c_obj_t *pOb = pyb_i2c_obj + ndx;
	LPI2C_MasterDeinit(pOb->pI2C);
	CLOCK_DisableClock(pOb->myClock);
	DisableIRQ(pOb->irqn);
}

void i2c_init_freq(pyb_i2c_obj_t *self, mp_int_t freq) {
    i2c_deinit(self->ndx);
	self->mstBaudrate = MP_OBJ_SMALL_INT_VALUE(freq);
	i2c_init(self->ndx, self->mstBaudrate);
}

STATIC void i2c_reset_after_error(pyb_i2c_obj_t *self) {
    // wait for bus-busy flag to be cleared, with a timeout
    for (int timeout = 50; timeout > 0; --timeout) {
        if (IS_MST_IDLE(self)) {
            return;
        }
        //mp_hal_delay_ms(1);
	i2c_release_bus_delay();
    }
    // bus was/is busy, need to reset the peripheral to get it to work again
    i2c_deinit(self->ndx);
    i2c_init(self->ndx, self->mstBaudrate);
}

/******************************************************************************/
/* Micro Python bindings                                                      */

STATIC void pyb_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_i2c_obj_t *self = self_in;

    uint i2c_num = self->ndx;

    if (!(self->flags & I2C_OBJ_FLAG_ENABLED)) {
        mp_printf(print, "I2C(%u)", i2c_num);
    } else {
        if (self->isMaster) {
            mp_printf(print, "I2C(%u, I2C.MASTER, baudrate=%u)", i2c_num, self->mstBaudrate);
        } else {
            mp_printf(print, "I2C(%u, I2C.SLAVE, addr=0x%02x)", i2c_num, self->slvCfg.address0 & 0x7f);
        }
    }
}

/// \method init(mode, *, addr=0x12, baudrate=400000, gencall=False)
///
/// Initialise the I2C bus with the given parameters:
///
///   - `mode` must be either `I2C.MASTER` or `I2C.SLAVE`
///   - `addr` is the 7-bit address (only sensible for a slave)
///   - `baudrate` is the SCL clock rate (only sensible for a master)
///   - `gencall` is whether to support general call mode
// make sure that you have init a i2c class success, use I2C(3), avoid that the self has a right value
STATIC mp_obj_t pyb_i2c_init_helper(pyb_i2c_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,     MP_ARG_INT, {.u_int = PYB_I2C_MASTER} },
        { MP_QSTR_addr,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0x12} },
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 400000} },
        { MP_QSTR_gencall,  MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_dma,      MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set the I2C configuration values, we just use the master mode , so we do not need to use the slave configure, but we add a slave configure_t in our self obj struct
	  self->slvCfg.address0 = (uint8_t)(args[1].u_int & 0x7F);
	  self->slvCfg.address1 = (uint8_t)((args[1].u_int << 1) & 0xFE);
	
    i2c_set_baudrate(self->ndx, MIN(args[2].u_int, MICROPY_HW_I2C_BAUDRATE_MAX));
	self->isUseDMA = args[4].u_bool;
	if (self->isUseDMA) {
		nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "DMA not yet supported"));

	}
    // init the I2C bus
    //i2c_deinit(self->ndx);
    i2c_init(self->ndx, args[2].u_int);
	self->flags |= I2C_OBJ_FLAG_ENABLED;
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct an I2C object on the given bus.  `bus` can be 1 or 2.
/// With no additional parameters, the I2C object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the I2C busses are:
///
///   - `I2C(1)` is on the X position: `(SCL, SDA) = (X9, X10) = (PB6, PB7)`
///   - `I2C(2)` is on the Y position: `(SCL, SDA) = (Y9, Y10) = (PB10, PB11)`
STATIC mp_obj_t pyb_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // work out i2c bus
    int i2c_id = 0;
    if (MP_OBJ_IS_STR(args[0])) {
        const char *port = mp_obj_str_get_str(args[0]);
        if (0) {
		#ifdef MICROPY_HW_I2C0_NAME
		} else if (strcmp(port, MICROPY_HW_I2C0_NAME) == 0) {
			i2c_id = PYB_I2C_0;
		#endif
		#ifdef MICROPY_HW_I2C1_NAME
		} else if (strcmp(port, MICROPY_HW_I2C1_NAME) == 0) {
			i2c_id = PYB_I2C_1;
		#endif		
		#ifdef MICROPY_HW_I2C2_NAME
		} else if (strcmp(port, MICROPY_HW_I2C2_NAME) == 0) {
			i2c_id = PYB_I2C_2;
		#endif
		#ifdef MICROPY_HW_I2C3_NAME
		} else if (strcmp(port, MICROPY_HW_I2C3_NAME) == 0) {
			i2c_id = PYB_I2C_3;
		#endif

        } else {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "I2C(%s) does not exist", port));
        }
    } else {
        i2c_id = mp_obj_get_int(args[0]);
        if (i2c_id > MP_ARRAY_SIZE(pyb_i2c_obj) || pyb_i2c_obj[i2c_id].pI2C == NULL) {
            nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError,
                "I2C(%d) does not exist", i2c_id));
        }
    }

    // get I2C object
    pyb_i2c_obj_t *i2c_obj = &pyb_i2c_obj[i2c_id ];

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        pyb_i2c_init_helper(i2c_obj, n_args - 1, args + 1, &kw_args);
    }

    return (mp_obj_t)i2c_obj;
}

STATIC mp_obj_t pyb_i2c_init(mp_uint_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return pyb_i2c_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_init_obj, 1, pyb_i2c_init);

/// \method deinit()
/// Turn off the I2C bus.
STATIC mp_obj_t pyb_i2c_deinit(mp_obj_t self_in) {
    pyb_i2c_obj_t *self = self_in;
    i2c_deinit(self->ndx);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_deinit_obj, pyb_i2c_deinit);
/// \method scan()
/// Scan all I2C addresses from 0x08 to 0x77 and return a list of those that respond.
/// Only valid when in master mode.
STATIC mp_obj_t pyb_i2c_scan(mp_obj_t self_in) {
    pyb_i2c_obj_t *self = self_in;

    if (!self->isMaster) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "I2C must be a master"));
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);

    for (uint addr = 0x02; addr <= 0x7E; addr++) {
	//	bool isOK = HAL_I2C_IsDeviceReady(self->pI2C, addr, 3, 200);
		if (1) {
			mp_obj_list_append(list, mp_obj_new_int(addr));
		}
    }

    return list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_i2c_scan_obj, pyb_i2c_scan);

/// \method send(send, addr=0x00, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object)
///   - `addr` is the address to send to (only required in master mode)
///   - `timeout` is the timeout in milliseconds to wait for the send
///
/// Return value: `None`.
STATIC mp_obj_t pyb_i2c_send(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_send,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_addr,    MP_ARG_INT, {.u_int = PYB_I2C_MASTER_ADDRESS} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_i2c_obj_t *self = pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(args[0].u_obj, &bufinfo, data);

    // if option is set and IRQs are enabled then we can use DMA
    bool use_dma = self->isUseDMA;// && query_irq() == IRQ_STATE_ENABLED;

	use_dma = use_dma;	// rocky todo: implement DMA

    // send the data
    status_t status;
    if (self->isMaster) {
        if (args[1].u_int == PYB_I2C_MASTER_ADDRESS) {
            if (use_dma) {
                use_dma = use_dma;	// rocky todo: implement DMA
            }
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "addr argument required"));
        }
        mp_uint_t i2c_addr = args[1].u_int;
        if (!use_dma) {
			while(1)
    		{
				status = LPI2C_MasterStart(self->pI2C, i2c_addr, kLPI2C_Write);

        		if (kStatus_Success != status)
        		{
            		LPI2C_MasterStop(self->pI2C);
	          		return (mp_obj_t)-1;
        		}
        		else
        		{
            		break;
        		}
    		}
			while (LPI2C_MasterGetStatusFlags(self->pI2C) & kLPI2C_MasterNackDetectFlag)
        	{
        	}
			if((status = LPI2C_MasterSend(self->pI2C, bufinfo.buf, 1)) != kStatus_Success)
				return status;
      } else {
            status = status; // todo: dma
        }
	} else {
		if (!use_dma) {
            status = status; // todo: slave
        } else {
            status = status; // todo: slave DMA
        }
    }

    // if we used DMA, wait for it to finish
    if (use_dma) {
        // todo: dma
    }

    if (status != kStatus_Success) {
        // i2c_reset_after_error(self);
        mp_hal_raise(status);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_send_obj, 1, pyb_i2c_send);

/// \method recv(recv, addr=0x00, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes
///   - `addr` is the address to receive from (only required in master mode)
///   - `timeout` is the timeout in milliseconds to wait for the receive
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC mp_obj_t pyb_i2c_recv(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_recv,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_addr,    MP_ARG_INT, {.u_int = PYB_I2C_MASTER_ADDRESS} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    };

    // parse args
    pyb_i2c_obj_t *self = pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // get the buffer to receive into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(args[0].u_obj, &vstr);

    // if option is set and IRQs are enabled then we can use DMA
    bool use_dma = self->isUseDMA;// && query_irq() == IRQ_STATE_ENABLED;

    if (use_dma) {
		use_dma = use_dma;
	}

    // receive the data
    status_t reVal;
    if (self->isMaster) {
        if (args[1].u_int == PYB_I2C_MASTER_ADDRESS) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "addr argument required"));
        }
        mp_uint_t i2c_addr = args[1].u_int;
        if (!use_dma) {
			while (1)
    		{
        		reVal = LPI2C_MasterStart(self->pI2C, i2c_addr, kLPI2C_Read);   //attention:in our read function,this will be the write,because i2c need send the control word to the salve kit before read.

        		if (kStatus_Success != reVal)
        		{
            		LPI2C_MasterStop(self->pI2C);
	    			return (mp_obj_t)-1;
        		}
        		else
        		{
            	break;
        		}
    		}
			while (LPI2C_MasterGetStatusFlags(self->pI2C) & kLPI2C_MasterNackDetectFlag)
        	{
       		}
			if((reVal = LPI2C_MasterReceive(self->pI2C, vstr.buf, vstr.len)) != kStatus_Success)
				return reVal;
			//before we set a i2c_stop here, but meet some erro, all because that the i2c on vega-board is special than RT, the receive must befind the send, no need to stop the i2c then re-start.
        	} else {
            // todo: DMA
        	}
    } else {
        if (!use_dma) {
            // todo: slave
        } else {
            // todo: slave DMA
        }
    }

    if (reVal != kStatus_Success) {
        i2c_reset_after_error(self);

        mp_hal_raise(reVal);
    }

    // return the received data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_recv_obj, 1, pyb_i2c_recv);

/// \method mem_read(data, addr, memaddr, timeout=5000, addr_size=8)
///
/// Read from the memory of an I2C device:
///
///   - `data` can be an integer or a buffer to read into
///   - `addr` is the I2C device address
///   - `memaddr` is the memory location within the I2C device
///   - `timeout` is the timeout in milliseconds to wait for the read
///   - `addr_size` selects width of memaddr: 8 or 16 bits
///
/// Returns the read data.
/// This is only valid in master mode.
STATIC const mp_arg_t pyb_i2c_mem_io_allowed_args[] = {
    { MP_QSTR_data,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_addr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_memaddr, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
    { MP_QSTR_addr_size, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
};

status_t pyb_i2c_mem_read_c(pyb_i2c_t ndx, bool use_dma, 
	mp_uint_t i2c_addr, mp_uint_t mem_addr, mp_uint_t mem_addr_size, char*buf, uint32_t cbRx)
{
    status_t reVal = kStatus_Fail;
	pyb_i2c_obj_t *self = pyb_i2c_obj + ndx;
    if (!use_dma) {
		    reVal = LPI2C_ReadBlock(self->pI2C, i2c_addr, mem_addr, buf, cbRx);
    } else {
        // todo: dma
    }
	return reVal;
}

STATIC mp_obj_t pyb_i2c_mem_read(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args
    pyb_i2c_obj_t *self = pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(pyb_i2c_mem_io_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(pyb_i2c_mem_io_allowed_args), pyb_i2c_mem_io_allowed_args, args);

    if (!self->isMaster) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "I2C must be a master"));
    }

    // get the buffer to read into
    vstr_t vstr;
    mp_obj_t o_ret = pyb_buf_get_for_recv(args[0].u_obj, &vstr);

    // get the addresses
    mp_uint_t i2c_addr = args[1].u_int;
    mp_uint_t mem_addr = args[2].u_int;
    // determine width of mem_addr; default is 8 bits, entering any other value gives 16 bit width
    mp_uint_t mem_addr_size = (args[4].u_int + 7) >> 3;

    // if option is set and IRQs are enabled then we can use DMA
    bool use_dma = self->isUseDMA;// && query_irq() == IRQ_STATE_ENABLED;

    status_t status = pyb_i2c_mem_read_c(self->ndx, 0, i2c_addr, mem_addr, mem_addr_size, vstr.buf, vstr.len);

    if (status != kStatus_Success) {
        i2c_reset_after_error(self);
        mp_hal_raise(status);
    }

    // return the read data
    if (o_ret != MP_OBJ_NULL) {
        return o_ret;
    } else {
        return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_mem_read_obj, 1, pyb_i2c_mem_read);

/// \method mem_write(data, addr, memaddr, timeout=5000, addr_size=8)
///
/// Write to the memory of an I2C device:
///
///   - `data` can be an integer or a buffer to write from
///   - `addr` is the I2C device address
///   - `memaddr` is the memory location within the I2C device
///   - `timeout` is the timeout in milliseconds to wait for the write
///   - `addr_size` selects width of memaddr: 8 or 16 bits
///
/// Returns `None`.
/// This is only valid in master mode.
STATIC mp_obj_t pyb_i2c_mem_write(mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args (same as mem_read)
    pyb_i2c_obj_t *self = pos_args[0];
    mp_arg_val_t args[MP_ARRAY_SIZE(pyb_i2c_mem_io_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(pyb_i2c_mem_io_allowed_args), pyb_i2c_mem_io_allowed_args, args);

    if (!self->isMaster) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError, "I2C must be a master"));
    }

    // get the buffer to write from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(args[0].u_obj, &bufinfo, data);

    // get the addresses
    mp_uint_t i2c_addr = args[1].u_int;
    mp_uint_t mem_addr = args[2].u_int;
    // determine width of mem_addr; default is 8 bits, entering any other value gives 16 bit width
    mp_uint_t mem_addr_size = 1;
    if (args[4].u_int != 8) {
        mem_addr_size = 2;
    }

    // if option is set and IRQs are enabled then we can use DMA
    bool use_dma = self->isUseDMA;// && query_irq() == IRQ_STATE_ENABLED;

    status_t status;
    if (!use_dma) {
     status = LPI2C_WriteBlock(self->pI2C, i2c_addr, mem_addr, bufinfo.buf, bufinfo.len);    	
    } else {
        // todo: dma
    }

    if (status != kStatus_Success) {
        i2c_reset_after_error(self);
        mp_hal_raise(status);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_i2c_mem_write_obj, 1, pyb_i2c_mem_write);

STATIC const mp_rom_map_elem_t pyb_i2c_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_i2c_deinit_obj) },
   // { MP_ROM_QSTR(MP_QSTR_is_ready), MP_ROM_PTR(&pyb_i2c_is_ready_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&pyb_i2c_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&pyb_i2c_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&pyb_i2c_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem_read), MP_ROM_PTR(&pyb_i2c_mem_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem_write), MP_ROM_PTR(&pyb_i2c_mem_write_obj) },

    // class constants
    /// \constant MASTER - for initialising the bus to master mode
    /// \constant SLAVE - for initialising the bus to slave mode
    { MP_ROM_QSTR(MP_QSTR_MASTER), MP_ROM_INT(PYB_I2C_MASTER) },
    { MP_ROM_QSTR(MP_QSTR_SLAVE), MP_ROM_INT(PYB_I2C_SLAVE) },
};

STATIC MP_DEFINE_CONST_DICT(pyb_i2c_locals_dict, pyb_i2c_locals_dict_table);

const mp_obj_type_t pyb_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .print = pyb_i2c_print,
    .make_new = pyb_i2c_make_new,
    .locals_dict = (mp_obj_dict_t*)&pyb_i2c_locals_dict,
};
