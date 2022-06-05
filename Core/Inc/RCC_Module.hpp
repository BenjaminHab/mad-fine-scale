#ifndef RCC_MODULE
#define RCC_MODULE

#include <main.h>
#include "Modules.hpp"


enum Clock_Source{
	HSI = 0b00,
	HSE = 0b01,
	PLL = 0b10
};

class RCC_Interface{
public:
	RCC_Interface();
	void set_system_clock(Clock_Source source_);
	void setup();
};











#endif
