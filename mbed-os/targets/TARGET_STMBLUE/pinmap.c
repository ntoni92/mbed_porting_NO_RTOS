/* mbed Microcontroller Library
 *******************************************************************************
 *******************************************************************************
 */
#include "mbed_assert.h"
#include "pinmap.h"
#include "bluenrg_x_device.h"


void pin_function(PinName pin, int data){
	uint8_t mode = data & 0xFF;
	if(pin < 8) {
				MODIFY_REG(GPIO->MODE0, (0xF<<(pin*4)), (mode << (pin*4)) );
			}
			else if((pin > 7) && (pin < 16)) {
				MODIFY_REG(GPIO->MODE1, (0xF<<((pin-8)*4)), (mode << ((pin-8)*4)) );
			}
		#ifdef BLUENRG2_DEVICE
			else if((pin > 15) && (pin < 24)) {
				MODIFY_REG(GPIO->MODE2, (0xF<<((pin-16)*4)), (mode << ((pin-16)*4)) );
			}
			else if(pin > 23) {
				MODIFY_REG(GPIO->MODE3, (0xF<<((pin-24)*4)), (mode << ((pin-24)*4)) );
			}
		#endif
}

#ifdef stm
#include "mbed_assert.h"
#include "pinmap.h"
#include "PortNames.h"
#include "mbed_error.h"
#include "pin_device.h"

extern GPIO_TypeDef *Set_GPIO_Clock(uint32_t port_idx);

const uint32_t ll_pin_defines[16] = {
    LL_GPIO_PIN_0,
    LL_GPIO_PIN_1,
    LL_GPIO_PIN_2,
    LL_GPIO_PIN_3,
    LL_GPIO_PIN_4,
    LL_GPIO_PIN_5,
    LL_GPIO_PIN_6,
    LL_GPIO_PIN_7,
    LL_GPIO_PIN_8,
    LL_GPIO_PIN_9,
    LL_GPIO_PIN_10,
    LL_GPIO_PIN_11,
    LL_GPIO_PIN_12,
    LL_GPIO_PIN_13,
    LL_GPIO_PIN_14,
    LL_GPIO_PIN_15
};

/**
 * Configure pin (mode, speed, output type and pull-up/pull-down)
 */
void pin_function(PinName pin, int data)
{
    MBED_ASSERT(pin != (PinName)NC);

    // Get the pin informations
    uint32_t mode  = STM_PIN_FUNCTION(data);
    uint32_t afnum = STM_PIN_AFNUM(data);
    uint32_t port = STM_PORT(pin);
    uint32_t ll_pin  = ll_pin_defines[STM_PIN(pin)];
    uint32_t ll_mode = 0;

    // Enable GPIO clock
    GPIO_TypeDef *gpio = Set_GPIO_Clock(port);

    /*  Set default speed to high.
     *  For most families there are dedicated registers so it is
     *  not so important, register can be set at any time.
     *  But for families like F1, speed only applies to output.
     */
#if defined (TARGET_STM32F1)
if (mode == STM_PIN_OUTPUT) {
#endif
    LL_GPIO_SetPinSpeed(gpio, ll_pin, LL_GPIO_SPEED_FREQ_HIGH);
#if defined (TARGET_STM32F1)
}
#endif

    switch (mode) {
        case STM_PIN_INPUT:
            ll_mode = LL_GPIO_MODE_INPUT;
            break;
        case STM_PIN_OUTPUT:
            ll_mode = LL_GPIO_MODE_OUTPUT;
            break;
        case STM_PIN_ALTERNATE:
            ll_mode = LL_GPIO_MODE_ALTERNATE;
            // In case of ALT function, also set he afnum
           stm_pin_SetAFPin(gpio, pin, afnum);
            break;
        case STM_PIN_ANALOG:
            ll_mode = LL_GPIO_MODE_ANALOG;
            break;
        default:
            MBED_ASSERT(0);
            break;
    }
    LL_GPIO_SetPinMode(gpio, ll_pin, ll_mode);

#if defined(GPIO_ASCR_ASC0)
    /*  For families where Analog Control ASC0 register is present */
    if (STM_PIN_ANALOG_CONTROL(data)) {
        LL_GPIO_EnablePinAnalogControl(gpio, ll_pin);
    } else {
        LL_GPIO_DisablePinAnalogControl(gpio, ll_pin);
    }
#endif

    /*  For now by default use Speed HIGH for output or alt modes */
    if ((mode == STM_PIN_OUTPUT) ||(mode == STM_PIN_ALTERNATE)) {
    if (STM_PIN_OD(data)) {
            LL_GPIO_SetPinOutputType(gpio, ll_pin, LL_GPIO_OUTPUT_OPENDRAIN);
    } else {
            LL_GPIO_SetPinOutputType(gpio, ll_pin, LL_GPIO_OUTPUT_PUSHPULL);
    }
    }

    stm_pin_PullConfig(gpio, ll_pin, STM_PIN_PUPD(data));

    stm_pin_DisconnectDebug(pin);
}

/**
 * Configure pin pull-up/pull-down
 */
void pin_mode(PinName pin, PinMode mode)
{
    MBED_ASSERT(pin != (PinName)NC);

    uint32_t port_index = STM_PORT(pin);
    uint32_t ll_pin  = ll_pin_defines[STM_PIN(pin)];
    // Enable GPIO clock
    GPIO_TypeDef *gpio = Set_GPIO_Clock(port_index);
    uint32_t function = LL_GPIO_GetPinMode(gpio, ll_pin);

    if ((function == LL_GPIO_MODE_OUTPUT) || (function == LL_GPIO_MODE_ALTERNATE))
    {
        if ((mode == OpenDrainNoPull) || (mode == OpenDrainPullUp) || (mode == OpenDrainPullDown)) {
            LL_GPIO_SetPinOutputType(gpio, ll_pin, LL_GPIO_OUTPUT_OPENDRAIN);
        } else {
            LL_GPIO_SetPinOutputType(gpio, ll_pin, LL_GPIO_OUTPUT_PUSHPULL);
        }
    }

    if ((mode == OpenDrainPullUp) || (mode == PullUp)) {
        stm_pin_PullConfig(gpio, ll_pin, GPIO_PULLUP);
    } else if ((mode == OpenDrainPullDown) || (mode == PullDown)) {
        stm_pin_PullConfig(gpio, ll_pin, GPIO_PULLDOWN);
    } else {
        stm_pin_PullConfig(gpio, ll_pin, GPIO_NOPULL);
    }
}
#endif