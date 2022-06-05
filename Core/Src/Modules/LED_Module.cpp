#include "LED_Module.hpp"

LED::LED(Output* pin_){
	pin = pin_;
	return;
}

void LED::setup(Output* pin_)
{
	pin = pin_;
    return;
}

void LED::on(){
    pin->set();
}

void LED::off(){
	pin->reset();
}
