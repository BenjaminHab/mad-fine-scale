//Define Library
#ifndef Statemachine_Module_hpp
#define Statemachine_Module_hpp

//Includes
#include <main.h>
#include "Menu_Module.hpp"
#include "Encoder_Module.hpp"
#include "Peripherals_Module.hpp"
#include <vector>
#include <functional>


//Types

//Functions
void error_func(uint8_t* active_state, Peripherals *peripherals);

//Classes
class State
{
public:
	//Constructors
	State();

	//Methods
	void enter(uint8_t* active_state, Peripherals *peripherals);

	//Attributes
	uint8_t id;

	//Pointers to state functions
	//    std::function<void(uint8_t* active_state, Peripherals *peripherals)> entry_action = [](uint8_t* active_state, Peripherals *peripherals){};
	//    std::function<void(uint8_t* active_state, Peripherals *peripherals)> body_action = [](uint8_t* active_state, Peripherals *peripherals){};
	//    std::function<void(uint8_t* active_state, Peripherals *peripherals)> exit_action = [](uint8_t* active_state, Peripherals *peripherals){};
	std::function<void(uint8_t* active_state, Peripherals *peripherals)> entry_action = &error_func;
	std::function<void(uint8_t* active_state, Peripherals *peripherals)> body_action = &error_func;
	std::function<void(uint8_t* active_state, Peripherals *peripherals)> exit_action = &error_func;
};

class Statemachine
{
public:
	//Constructors
	Statemachine();

	//Methods
	void enter();
	void setup();

	//Attributes
	//    std::vector<State*> *states = nullptr;
	State** states = nullptr;
	uint8_t active_state = 0;

	//Linked Resources
	Peripherals peripherals;


};



#endif
