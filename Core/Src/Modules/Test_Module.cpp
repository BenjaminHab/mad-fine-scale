#include <main.h>
#include "Test_Module.hpp"
#include "Modules.hpp"

Test::Test(){

}

Display_Test::Display_Test(){
	static Display display;
	peripherals.display = &display;
	peripherals.display->setup(u8x8_byte_hw_i2c, u8x8_stm32_gpio_and_delay);

}

void Display_Test::perform(){
	u8g2_FirstPage(peripherals.display->u8g2);
	do
	{	u8g2_DrawStr(peripherals.display->u8g2, 4, 9, column_1_text);
//		u8g2_DrawStr(peripherals.display->u8g2, 4, 21, column_1_str);
		u8g2_DrawStr(peripherals.display->u8g2, 44, 9, column_2_text);
//		u8g2_DrawStr(peripherals.display->u8g2, 44, 21, column_2_str);

	} while (u8g2_NextPage(peripherals.display->u8g2));




		HAL_Delay(50);
		return;
}

ADC_Test::ADC_Test(){

	//static Test adc_test;
	//static Peripherals peripherals;
	static ADCobj adc;
	peripherals.adc = &adc;
	peripherals.adc->setup();

	static Display display;
	peripherals.display = &display;
	peripherals.display->setup(u8x8_byte_hw_i2c, u8x8_stm32_gpio_and_delay);

	static char column_1_str_data[] = "0000";
	column_1_str  = column_1_str_data;
	static char column_2_str_data[] = "0000";
	column_2_str  = column_2_str_data;
	static char column_3_str_data[] = "0000";
	column_3_str  = column_3_str_data;

}

void ADC_Test::perform(){
	//Read both channels
	column_1_read = peripherals.adc->read(TEMP_CHANNEL);
	column_2_read = peripherals.adc->read(CURRENT_CHANNEL);
	column_3_read = peripherals.adc->read(VOLTAGE_CHANNEL);

	//convert to text
	uint_to_cstr(column_1_read, column_1_str);
	uint_to_cstr(column_2_read, column_2_str);
	uint_to_cstr(column_3_read, column_3_str);

	u8g2_FirstPage(peripherals.display->u8g2);
	do
	{

		u8g2_DrawStr(peripherals.display->u8g2, 4, 9, column_1_text);
//		u8g2_DrawStr(peripherals.display->u8g2, 4, 21, column_1_str);
		u8g2_DrawStr(peripherals.display->u8g2, 44, 9, column_2_text);
//		u8g2_DrawStr(peripherals.display->u8g2, 44, 21, column_2_str);
		u8g2_DrawStr(peripherals.display->u8g2, 84, 9, column_3_text);
//		u8g2_DrawStr(peripherals.display->u8g2, 84, 21, column_3_str);

	} while (u8g2_NextPage(peripherals.display->u8g2));




	HAL_Delay(50);
	return;
}


Menu_Test::Menu_Test(){
	static Encoder encoder;
	encoder.setup();
	peripherals.encoder = &encoder;

	static Display display;
	display.setup(&u8x8_byte_hw_i2c, &u8x8_stm32_gpio_and_delay);
	peripherals.display = &display;

	static Menu menu(peripherals.encoder, peripherals.display);
	menu.setup();
	peripherals.menu = &menu;
}

void Menu_Test::perform(){
	peripherals.menu->enter();
	HAL_Delay(10);
}


/* Test the timer for pulse generation.
 * Setup: Breakout Board with attached logic analyzer
 * Expectation:
 * Measure a 2ms pulse every 500ms on PA4
 * PASSED */

Pulse_Test::Pulse_Test(){
	//setup display
	static Display display;
	display.setup(&u8x8_byte_hw_i2c, &u8x8_stm32_gpio_and_delay);
	peripherals.display = &display;

	//Set PA4 to AF mode, pull down, AF4 (TIM14 OC)
	AFIO pulsepin(GPIOA, 4, PULL_DOWN, AF4);
	pulsepin.set_speed(HIGH_SPEED);

	//Setup Timer 14:
	RCC->APBENR2 |= RCC_APBENR2_TIM14EN; //Enable peripheral clock for TIM14
	TIM14->CR1 &= ~(0b11 << TIM_CR1_CKD_Pos); //Clock division 1
	//	TIM14->PSC = 23999; //-> 0.5ms clock period
	TIM14->PSC = 4799; //-> 0.1ms clock period


	//	TIM14->CCMR1 |= TIM_CCMR1_OC1FE; //Enable fast mode
	//	TIM14->CCER &= ~(TIM_CCER_CC1P); //Output polarity active high, reset value
	TIM14->CCER |= TIM_CCER_CC1E; //Enable capture compare output
	//	TIM14->OR //Check this register if the rest does not work, maybe the solution lies here


	//PWM Mode 1 and CCIF to disable:
	TIM14->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); //Set PWM mode 1
	TIM14->CCR1 = 20; //pulsetime in 0.1ms
	TIM14->ARR = 0xffff; //long remaining time

}

void Pulse_Test::perform(){

	TIM14->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF); //clear update and capture/compare interrupt flags
	TIM14->CNT = 0; //clear counter
	TIM14->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos); //reset OC mode
	TIM14->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); //set pwm mode 1
	TIM14->CR1 |= TIM_CR1_CEN; //Enable counter a.k.a. start timer
	//	char high[] = "HIGH";
	//	peripherals.display->print_state(high);
	while(!(((TIM14->SR) & TIM_SR_CC1IF)>0)){ //wait for timer to finish

	}
	HAL_Delay(100);
	TIM14->CR1 &= ~TIM_CR1_CEN; //Stop timer

	TIM14->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos); //reset OC mode
	TIM14->CCMR1 |= (0b100 << TIM_CCMR1_OC1M_Pos); //force inactive mode
	//Test if OPM is working
	//	HAL_Delay(100);
	//	if(TIM14->CR1 & TIM_CR1_CEN){
	//		char success[] = "success";
	//		peripherals.display->print_state(success);
	//	}else{
	//		char failure[] = "failure";
	//		peripherals.display->print_state(failure);
	//	}
	//	char low[] = "LOW?";
	//	peripherals.display->print_state(low);
	HAL_Delay(500);
}

/* Test the process of arming, lifting the electrode and arc ignition
 * Setup:
 * Logic PCB and connected Power PCB, lab-bench power supply with capacitor
 * Expectation:
 * Generate a pulse, when armed and electrode is lifted
 * PENDING */

Initiate_Pulse_Test::Initiate_Pulse_Test(){
	//setup display
	static Display display_;
	display_.setup(&u8x8_byte_hw_i2c, &u8x8_stm32_gpio_and_delay);
	peripherals.display = &display_;

	//setup ADC
	static ADCobj adc_;
	adc_.setup();
	peripherals.adc = &adc_;

	//Setup OC
	AFIO pulsepin(GPIOA, 4, PULL_DOWN, AF4);
	pulsepin.set_speed(HIGH_SPEED);

	//Setup Timer 14:
	RCC->APBENR2 |= RCC_APBENR2_TIM14EN; //Enable peripheral clock for TIM14
	//	TIM14->CR1 &= ~(0b11 << TIM_CR1_CKD_Pos); //Clock division 1, reset value
	//	TIM14->PSC = 23999; //-> 0.5ms clock period
	TIM14->PSC = 4799; //-> 0.1ms clock period
	//	TIM14->CCMR1 |= TIM_CCMR1_OC1FE; //Enable fast mode
	//	TIM14->CCER &= ~(TIM_CCER_CC1P); //Output polarity active high, reset value
	TIM14->CCER |= TIM_CCER_CC1E; //Enable capture compare output
	//	TIM14->OR //Check this register if the rest does not work, maybe the solution lies here
	//PWM Mode 1, disable timer on CCIF
	TIM14->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); //Set PWM mode 1, i.e. Output active while CNT < CCR1, else inactive
	//TIM14->CCR1 = 20; //pulsetime in 0.1ms
	TIM14->ARR = 0xffff; //long remaining time

	//setup ARM pin

	arm = Input(GPIOF, 0, PULL_UP);

	pulsetime_setting.scale = 10;
	pulsetime_setting.setting_value = 20;
}

void Initiate_Pulse_Test::perform(){
//	peripherals.display->print_state("Idle");
//	if((arm.read()==LOW) && (peripherals.adc->read(TEST_CURRENT)>1024)){ //check if arm button depressed AND electrode has contact
//		peripherals.adc->vref = peripherals.adc->vsense; //save last adc reading
//		peripherals.display->print_state("Arming");
//		while(arm.read() == LOW){} //Wait for button release
//		while((peripherals.adc->read(TEST_CURRENT)*4) > (peripherals.adc->vref)){} //wait for the current to drop
//		if(peripherals.adc->vsense > 127){ //if the current has dropped and is not nearly zero, fire pulse
//			weld_pulse(&pulsetime_setting);
//			peripherals.display->print_state("Firing");
//			HAL_Delay(500);
//		}
//	}
//	HAL_Delay(10);
	return;
}

Statemachine_Test::Statemachine_Test(){
	//init statemachine...
	sm = Statemachine();
	sm.setup();
	return;
}

void Statemachine_Test::perform(){
	sm.enter();
	return;
}


