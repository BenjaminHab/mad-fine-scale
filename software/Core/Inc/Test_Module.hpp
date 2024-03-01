#ifndef Test_Module_hpp
#define Test_Module_hpp

#include "ADC_Module.hpp"
#include "Menu_Module.hpp"
#include "Display_Module.hpp"
#include "Encoder_Module.hpp"
#include "GPIO_Module.hpp"
#include "Init_Modules.hpp"
#include "LED_Module.hpp"
#include "Peripherals_Module.hpp"
#include "Statemachine_Module.hpp"

class Test
{
public:
	Test();
	Peripherals peripherals;
};

class Display_Test: public Test{
public:
	Display_Test();

	void perform();

	const char column_1_text[5] = "Test";
	const char column_2_text[5] = "Disp";
};

class ADC_Test: public Test{
public:
	ADC_Test();

	void perform();

	char* column_1_str;
	char* column_2_str;
	char* column_3_str;
	uint16_t column_1_read;
	uint16_t column_2_read;
	uint16_t column_3_read;
	const char column_1_text[5] = "Temp";
	const char column_2_text[5] = "Load";
	const char column_3_text[5] = "Batt";
};

class Menu_Test: public Test{
public:
	Menu_Test();
	//void setup();
	void perform();
};

class Pulse_Test: public Test{
public:
	Pulse_Test();
	void perform();
};

class Initiate_Pulse_Test : public Test{
public:
	Initiate_Pulse_Test();
	void perform();
	Input arm;
	setting_t pulsetime_setting;
};

class Statemachine_Test : public Test{
	public:
	Statemachine_Test();
	void perform();
	Statemachine sm;
};


#endif
