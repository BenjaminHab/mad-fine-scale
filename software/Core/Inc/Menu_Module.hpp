//Define Library
#ifndef Menu_Module_hpp
#define Menu_Module_hpp

//Includes
#include <main.h>
#include "Display_Module.hpp"
#include "Encoder_Module.hpp"
#include <map>


//Defines

//Variables

//Types
//Todo: Replace Types with classes

namespace menu{
enum state_t : unsigned char {
	MENU_STATE, 
	SETTING_STATE//,
	//EXIT_STATE
};
}

typedef struct submenu {
	struct submenu* next;
	struct submenu* previous;
	struct submenu* up;
	struct setting* assigned_setting;
	char* name;
}submenu_t;

typedef struct setting{
	submenu_t* up;
	uint8_t setting_value;
	uint8_t upper_bound;
	uint8_t lower_bound;
	uint8_t scale;
	char* unit;
} setting_t;

//Classes
//Define dependent classes for compiler
class Encoder;
class Display;


class Menu
{

public:
	//Constructors
	Menu(Encoder* encoder_  = nullptr, Display* display_  = nullptr);


	//other methods
	void setup();
	void enter();

	//variables:
	submenu_t *menu_entry = nullptr;
	submenu_t *current_menu = nullptr;
	setting_t *current_setting = nullptr;
	menu::state_t menu_state = menu::MENU_STATE;
	Encoder *encoder = nullptr;
	Display *display = nullptr;
	int8_t encoder_increment = 0;
	uint8_t encoder_button = 0;

	//Dictionary with setting pointers
	std::map<int, setting_t*> settings; //TODO: Try this version

};


//Functions
//TODO: Replace linked list with std::Array/std::Vector/std::Map? of structs/objects
void insert_menu(submenu_t* menu_element, char* element_name, submenu_t* previous_element, submenu_t* next_element, submenu_t* up_element, setting_t* assigned_setting );
void append_menu(submenu_t* menu_element, char* element_name, submenu_t* previous_element, setting_t* assigned_setting);
void close_menu_circle(submenu_t* first_element, submenu_t* up_element);

#endif
