/*
This is the connecting piece of all Modules. 
It is the place, where the nonportable parts are to be defined and implemented - 
what should happen in each state, what pins are used for the LED etc. 
The goal is to have everything fully portable except Init_Modules.hpp and Init_Modules.cpp
and to be able to write into main:

Statemachine statemachine;
statemachine.begin()

 */

#ifndef Init_Modules_hpp
#define Init_Modules_hpp

//Includes
#include "Encoder_Module.hpp"
#include "Menu_Module.hpp"
#include "Display_Module.hpp"
#include "Statemachine_Module.hpp"
#include <vector>
#include "u8g2.h"


#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
//Defines
// #define IDLE_STATE 0U
// #define ARMED_STATE 1U

//Types
namespace statemachine
{
enum state_t : uint8_t
{
	IDLE_STATE,
	PRESSED_STATE,
	ARMED_STATE,
	MENU_STATE,
	DEBUG_STATE
};
}

namespace menu
{
enum setting_id : uint8_t
{
	PULSETIME_SETTING,
	ARM_DELAY_SETTING

};
}

//Menu

//Statemachine

//Encoder

//ADC

//further functions

//uint8_t read_arm_button();

void weld_pulse(setting_t *pulsetime_setting);

void setup_OC();

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,
		U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,
		U8X8_UNUSED void *arg_ptr);

#endif
