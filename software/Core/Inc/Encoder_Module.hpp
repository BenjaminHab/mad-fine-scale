//Define Library
#ifndef Encoder_Module_hpp
#define Encoder_Module_hpp

//Includes
#include <main.h>
#include "GPIO_Module.hpp"

//Defines

//Variables

//Classes
class Encoder
{
public:
	//Constructor
	Encoder();
	//void setup{};
	int8_t increment();
	uint8_t button();
	void setup();
	Input* button_pin;

//	void setup_Button();
//	void setup_Timer();
	uint8_t button_action = 0;
//	GPIO_TypeDef* Button_Port = ENCODER_BUTTON_GPIO_Port;
//	uint16_t Button_Pin = ENCODER_BUTTON_Pin;

};

#endif
