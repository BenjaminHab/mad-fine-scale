#ifndef LED_Module_hpp
#define LED_Module_hpp

#include <main.h>
#include "GPIO_Module.hpp"

class LED
{
public:
    //Constructor
    LED(Output* pin_);

    Output* pin;
    void setup(Output* pin_);
    void on();
    void off();
};

#endif
