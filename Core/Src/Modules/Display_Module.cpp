
#include <main.h>
#include "Display_Module.hpp"
#include "u8g2.h"
//#include "stm32f0xx_hal.h"
#include "i2c.h"
#include "Menu_Module.hpp"

Display::Display()
{
	static u8g2_t u8g2_instance;
	u8g2 = &u8g2_instance;
	return;
}

Display::Display(u8g2_t* u8g2_pointer){
	u8g2 = u8g2_pointer;
	return;
}

void Display::print_state(char *current_state)
{
	u8g2_FirstPage(u8g2);
	do
	{
		u8g2_DrawStr(u8g2, 19, 9, "State");
		u8g2_DrawStr(u8g2, 25, 21, current_state);
	} while (u8g2_NextPage(u8g2));
}

//void Display::print_menu(state_t *state, submenu_t **current_menu, setting_t **current_setting, int8_t encoder_increment, uint8_t button_action)
void Display::print_menu(Menu menu)
{
	//Display Menu
	//u8g2_SetFont(&u8g2, u8g2_font_missingplanet_tr); //u8g2_font_nine_by_five_nbp_t_all
	// u8g2_FirstPage(&u8g2);
	// do
	// {
	// 	u8g2_DrawStr(&u8g2, 19, 9, ((*current_menu)->previous->name));
	// 	u8g2_DrawStr(&u8g2, 25, 21, ((*current_menu)->name));
	// 	u8g2_DrawStr(&u8g2, 20, 32, ((*current_menu)->next->name));
	// 	u8g2_DrawCircle(&u8g2, 0, 16, 16, U8G2_DRAW_ALL);
	// 	u8g2_DrawTriangle(&u8g2, 127, 14, 127, 20, 117, 17);
	// } while (u8g2_NextPage(&u8g2));

	u8g2_FirstPage(u8g2);
	do
	{
		u8g2_DrawStr(u8g2, 19, 9, ((menu.current_menu)->previous->name));
		u8g2_DrawStr(u8g2, 25, 21, ((menu.current_menu)->name));
		u8g2_DrawStr(u8g2, 20, 32, ((menu.current_menu)->next->name));
		u8g2_DrawCircle(u8g2, 0, 16, 16, U8G2_DRAW_ALL);
		u8g2_DrawTriangle(u8g2, 127, 14, 127, 20, 117, 17);
	} while (u8g2_NextPage(u8g2));


	return;
}

// void Display::print_setting(state_t *state, submenu_t **current_menu, setting_t **current_setting, int8_t encoder_increment, uint8_t button_action)
void Display::print_setting(Menu menu, char* value)
{
	// uint8_t numval = (menu.current_setting)->setting_value;
	// char value[4];
	// //sprintf(value, "%d", (*(*current_setting))->setting_value);

	// for (uint8_t i = 2; i <= 4; i++)
	// {
	// 	value[4 - i] = (char)((numval % 10U) + '0');
	// 	numval /= 10;
	// }
	// value[3] = '\0';

	// if (encoder_increment != 0)
	// {
	// 	uint8_t i = ((*current_setting)->setting_value - encoder_increment * ((*current_setting)->scale));
	// 	if ((i <= ((*current_setting)->upper_bound)) && (i >= ((*current_setting)->lower_bound)))
	// 	{
	// 		(*current_setting)->setting_value = i;
	// 	}
	// }
	// else if (button_action)
	// {
	// 	*state = MENU_STATE;
	// }
	u8g2_FirstPage(u8g2);
	do
	{
		u8g2_DrawStr(u8g2, 19, 9, ((menu.current_menu)->name));
		u8g2_DrawStr(u8g2, 25, 21, value);
		u8g2_DrawStr(u8g2, 100, 21, (menu.current_setting)->unit);
	} while (u8g2_NextPage(u8g2));
}

void Display::print_value(uint16_t numval){
	//Assemble a string to print
	char value[5];
	for (uint8_t i = 2; i <= 5; i++)
	{
		value[5 - i] = (char)((numval % 10U) + '0');
		numval /= 10;
	}
	value[4] = '\0';
	//	const char test[] = "3456";
	u8g2_FirstPage(u8g2);
	do
	{
		u8g2_DrawStr(u8g2, 25, 21, value);
		//		u8g2_DrawStr(u8g2, 25, 21, test);
	} while (u8g2_NextPage(u8g2));
	return;
}

void uint_to_cstr(uint16_t numval, char* text){
	//Assemble a string to print
		for (uint8_t i = 2; i <= 5; i++)
		{
			text[5 - i] = (char)((numval % 10U) + '0');
			numval /= 10;
		}
		text[4] = '\0';
		return;
}



