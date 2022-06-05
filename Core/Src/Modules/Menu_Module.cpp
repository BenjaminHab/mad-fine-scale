
#include <main.h>
#include "Menu_Module.hpp"
// #include "menu.h"
//#include "stm32f0xx_hal.h"
#include "i2c.h"
#include "Encoder_Module.hpp"
#include "Display_Module.hpp"

using namespace menu;

Menu::Menu(Encoder* encoder_, Display* display_)
{
	if(encoder_){
		encoder = encoder_;

	}else{
		static Encoder encoder_instance;
		encoder = &encoder_instance;
	}
	if(display_){
		display = display_;
	}else{
		static Display display_instance;
		display = &display_instance;

	}
	//Connect peripherals

}

void Menu::enter(){
	menu_state = MENU_STATE;
	while (true){
		encoder_increment = (*encoder).increment();
		encoder_button = (*encoder).button();
		switch (menu_state) {
		case MENU_STATE:{
			display->print_menu((*this));
			//Read button action
			switch (encoder_button) {
			case 0: {//No button press, check rotation
				//TODO: Shorten code
				if (encoder_increment < 0){
					current_menu = current_menu->previous;
					//				encoder_increment++;
				}
				else if (encoder_increment > 0){
					current_menu = current_menu->next;
					//				encoder_increment--;
				}
				break;
			}
			case 1: {//short button press, ignore rotation
				if (current_menu->assigned_setting == NULL){ //Up-Element
					return;
				}
				current_setting = (current_menu)->assigned_setting;
				menu_state = SETTING_STATE;
				break;
			}
			case 2:{ //long button press, ignore rotation
				if (current_menu->up == NULL){
					return;
				}
				current_menu = current_menu->up;
				break;
			}
			}
			break;
		}
		case SETTING_STATE: {
			if(encoder_button){ //Leave Setting
				menu_state = MENU_STATE;
				break;
			}else if (encoder_increment != 0){ //Change the current setting according to encoder input
				uint8_t i = (current_setting->setting_value - encoder_increment * (current_setting->scale)); //TODO: Fix this mess with a uint8_t being possibly assigned a negative value
				if ((i <= (current_setting->upper_bound)) && (i >= (current_setting->lower_bound))){
					current_setting->setting_value = i;
				}
			}

			//Prepare to print current value
			uint8_t numval = (current_setting)->setting_value;
			char value[4];
			for (uint8_t i = 2; i <= 4; i++){
				value[4 - i] = (char)((numval % 10U) + '0');
				numval /= 10;
			}
			value[3] = '\0';
			display->print_setting((*this), value);
			break;
		}
		default:
			return;
		}
//		HAL_Delay(10);
	}
}

//Helper functions for linked list creation of submenus
void insert_menu(submenu_t *menu_element, char *element_name, submenu_t *previous_element, submenu_t *next_element, submenu_t *up_element, setting_t *linked_setting)
{

	//assign name
	menu_element->name = element_name;

	//setup pointers in new element
	menu_element->previous = previous_element;
	menu_element->next = next_element;

	menu_element->assigned_setting = linked_setting;
	menu_element->up = up_element;

	//modify previous/next element and setting
	if (previous_element != NULL)
	{
		previous_element->next = menu_element;
	}
	if (next_element != NULL)
	{
		next_element->previous = menu_element;
	}
	if (linked_setting != NULL)
	{
		linked_setting->up = menu_element;
	}
	return;
}

void append_menu(submenu_t *menu_element, char *element_name, submenu_t *previous_element, setting_t *assigned_setting)
{
	insert_menu(menu_element, element_name, previous_element, NULL, previous_element->up, assigned_setting);
	return;
}

void close_menu_circle(submenu_t *first_element, submenu_t *up_element)
{
	submenu_t *current_element = first_element;
	while (current_element->next != NULL)
	{
		current_element = current_element->next;
	}

	static char text[] = "Up";
	insert_menu(up_element, text, current_element, first_element, first_element->up, NULL);
	return;
}
//
