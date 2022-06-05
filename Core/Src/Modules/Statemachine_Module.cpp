
#include <main.h>
#include "Statemachine_Module.hpp"


void error_func(uint8_t *active_state, Peripherals *peripherals)
{
	peripherals->display->print_state("Error");
	while(true){} //Wait for user to notice
	return;
}


Statemachine::Statemachine()
{
	return;
}

void Statemachine::enter()
{

	while (true)
	{
		//Eternal loop: Enter states as they come
		(states[active_state])->enter(&active_state, &peripherals);
	}
	return;
}

State::State()
{
	return;
}

void State::enter(uint8_t *active_state, Peripherals *peripherals)
{   
	entry_action(active_state, peripherals);
	while (*active_state == id)
	{
		body_action(active_state, peripherals);
	}
	exit_action(active_state, peripherals);
	return;
}

