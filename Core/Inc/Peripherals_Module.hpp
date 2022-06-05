#ifndef Peripherals_Module_hpp
#define Peripherals_Module_hpp

#include <main.h>
#include "ADC_Module.hpp"
#include "Display_Module.hpp"
#include "Encoder_Module.hpp"
#include "LED_Module.hpp"

class Peripherals
{
public:
//    Peripherals();

    LED *led = nullptr;
    ADCobj *adc = nullptr;
    Display *display = nullptr;
    Encoder *encoder = nullptr;
    Menu *menu = nullptr;
    Input *button = nullptr;
};

#endif
