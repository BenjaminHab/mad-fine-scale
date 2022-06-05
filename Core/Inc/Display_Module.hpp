//Define Library
#ifndef Display_Module_hpp
#define Display_Module_hpp

//Includes
#include <main.h>
#include "u8g2.h"
#include "Menu_Module.hpp"
//Defines

//Variables

//Classes
//Define dependent class Menu for compiler
class Menu;

class Display
{
public:
	//Constructor
	Display();
	Display(u8g2_t* u8g2_pointer);
	//void setup{};
	void print_state(char *current_state);
	void print_menu(Menu menu);
	void print_setting(Menu menu, char* value);
	void print_value(uint16_t numval);
	void setup(u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb);

//private:
	u8g2_t* u8g2;
};

void uint_to_cstr(uint16_t numval, char* text);

#endif
