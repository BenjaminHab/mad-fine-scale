#include <main.h>
#include <vector>
#include <array>
//#include "Init_Modules.hpp"
//#include "ADC_Module.hpp"
//#include "Peripherals_Module.hpp"
//#include "GPIO_Module.hpp"
#include "Modules.hpp"

//TODO: Init ADC myself

void Menu::setup()
{
	//Save names and submenus
	static char pulsetime_name[] = "Pulsetime";
	static setting_t pulsetime_setting;
	static submenu_t pulsetime_menu;

	static char ARM_delay_name[] = "ARM Delay";
	static setting_t ARM_delay_setting;
	static char unit_ms[] = "ms";
	static submenu_t ARM_delay_menu;

	static submenu_t up_element;

	//TODO: Replace linked list with std::Array
	//Setup the menu
	insert_menu(&pulsetime_menu, pulsetime_name, NULL, NULL, NULL, &pulsetime_setting);
	append_menu(&ARM_delay_menu, ARM_delay_name, &pulsetime_menu, &ARM_delay_setting);
	close_menu_circle(&pulsetime_menu, &up_element);
	menu_entry = &pulsetime_menu;
	current_menu = menu_entry;

	//Set boundaries for the settings
	ARM_delay_setting.lower_bound = 0;
	ARM_delay_setting.upper_bound = 100;
	ARM_delay_setting.scale = 1;
	ARM_delay_setting.setting_value = 0;
	ARM_delay_setting.unit = unit_ms;

	pulsetime_setting.lower_bound = 0;
	pulsetime_setting.upper_bound = 40;
	pulsetime_setting.scale = 10;
	pulsetime_setting.setting_value = 1;
	pulsetime_setting.unit = unit_ms;
	//TODO: Add ADC Threshold setting

	//Gather settings in pointer map, so they can be modified easier
	settings[menu::PULSETIME_SETTING] = &pulsetime_setting;
	settings[menu::ARM_DELAY_SETTING] = &ARM_delay_setting;

	return;
}


void Display::setup(u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
	u8g2_Setup_ssd1306_i2c_128x32_univision_2(u8g2, U8G2_R0, (u8x8_byte_hw_i2c), (u8x8_stm32_gpio_and_delay));
	//u8g2_Setup_ssd1306_i2c_128x64_noname_1(u8g2, U8G2_R0, (u8x8_byte_hw_i2c), (u8x8_stm32_gpio_and_delay));
	HAL_Delay(500);
	u8g2_InitDisplay(u8g2);
	HAL_Delay(500);
	u8g2_SetPowerSave(u8g2, 0);
	HAL_Delay(1000);
	u8g2_ClearDisplay(u8g2);
	u8g2_SetContrast(u8g2, 120);
	u8g2_SetFont(u8g2, u8g2_font_missingplanet_tr);
}

void Encoder::setup()
{

	//Button setup
	static Input encoder_button_pin(GPIOA, 5, PULL_UP); 	// Initialize GPIO
	button_pin = &encoder_button_pin;

	//Setup Button External Interrupt Edge detection
//	SYSCFG->EXTICR[1] &= ~(0b1111 << SYSCFG_EXTICR2_EXTI5_Pos); // Set EXTI5 to watch pin A5
	//	EXTI->EMR |= (EXTI_EMR_EM5); // Unmask the Event
	EXTI->IMR1 |= (EXTI_IMR1_IM5); // Unmask the Interrupt
	EXTI->RTSR1 = (EXTI_RTSR1_RT5); // Setup Trigger on rising edge
	EXTI->FTSR1 = (EXTI_FTSR1_FT5); // Setup Trigger on falling edge
	EXTI->SWIER1 = EXTI_SWIER1_SWI5; // Set Software Event, so that event can be read from EXTI->PR

	//Setup timer 16
	RCC->APBENR2 |= RCC_APBENR2_TIM16EN; //Enable TIM16 peripheral clock
	TIM16->PSC = 47999; //Set prescaler so one tick is equivalent to 1ms
	TIM16->ARR = 4999; //Stop timer after 5 seconds
	TIM16->CR1 |= TIM_CR1_OPM;


	AFIO encoder_pin_a(GPIOA, 6, PULL_UP, AF1);
	//	encoder_pin_a.set_speed(HIGH_SPEED);
	AFIO encoder_pin_b(GPIOA, 7, PULL_UP, AF1);
	//	encoder_pin_b.set_speed(HIGH_SPEED);

	// Timer 3 setup
	RCC->APBENR1 |= RCC_APBENR1_TIM3EN;// Enable TIM3 peripheral clock
	TIM3->ARR = 0xffff; //set Auto Reload Register to a high value, so it won't make a problem
	TIM3->CCMR1 |= (0b10 << TIM_CCMR1_CC1S_Pos) | (0b10 << TIM_CCMR1_CC2S_Pos); // Map the input pins to TI1 and TI2, respectively
	//	TIM3->SMCR |= (0b0101 << TIM_SMCR_ETF_Pos); //Set intermediate filter???
	TIM3->CCMR1 |= (0b1111 << TIM_CCMR1_IC1F_Pos) | (0b1111 << TIM_CCMR1_IC2F_Pos); //Set input filters for both inputs
	TIM3->SMCR |= TIM_SMCR_SMS_0; //count on TI1 edges only
	TIM3->CNT = 100; //Preset the counter
	TIM3->CR1 |= TIM_CR1_CEN; 	//enable counter

	return;
}

void ADCobj::setup()
{ //Mostly stolen from snippets
	//TODO: Setup Pin

	//Setup Clock
	/* (1) Enable the peripheral clock of the ADC */
	/* (2) Start HSI14 RC oscillator */
	/* (3) Wait HSI14 is ready */
	RCC->APBENR2 |= RCC_APBENR2_ADCEN;        /* (1) */
	RCC->CR |= RCC_CR_HSION;               /* (2) */
	while ((RCC->CR & RCC_CR_HSIRDY) == 0) /* (3) */
	{
		/* For robust implementation, add here time-out management */
	}

	//Calibrate ADC
	/* (1) Ensure that ADEN = 0 */
	/* (2) Clear ADEN */
	/* (3) Launch the calibration by setting ADCAL */
	/* (4) Wait until ADCAL=0 */
	// if ((ADC1->CR & ADC_CR_ADEN) != 0) /* (1) */
	// {
	ADC1->CR &= (uint32_t)(~ADC_CR_ADEN); /* (2) */
	// }
	while ((ADC1->CR & ADC_CR_ADEN)){} //Wait for ADC to be off
	ADC1->CR |= ADC_CR_ADCAL;              /* (3) */
	while ((ADC1->CR & ADC_CR_ADCAL) != 0) /* (4) */ //FIXME: Seemingly endless loop
	{
		/* For robust implementation, add here time-out management */
	}

	//Configure ADC
	/* (1) Select HSI14 by writing 00 in CKMODE (reset value) */
	/* (2) Select the auto off mode */
	/* (3) Select CHSEL17 for VRefInt */
	/* (4) Select a sampling mode of 111 i.e. 239.5 ADC clk to be greater than 17.1us */
	/* (5) Wake-up the VREFINT (only for VBAT, Temp sensor and VRefInt) */
	//ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE; /* (1) */
	//ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;                                /* (2) */
	// ADC1->CHSELR = ADC_CHSELR_CHSEL17;                              /* (3) */
	ADC1->SMPR |= ADC_SMPR_SMP1_0 | ADC_SMPR_SMP1_1 | ADC_SMPR_SMP1_2; /* (4) */
	// ADC->CCR |= ADC_CCR_VREFEN;                                     /* (5) */

	//Enable ADC
	/* (1) Enable the ADC */
	/* (2) Wait until ADC ready */
	do
	{
		/* For robust implementation, add here time-out management */
		ADC1->CR |= ADC_CR_ADEN; /* (1) */
	} while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) /* (2) */;

	return;
}


//void GPIO_setup(){
//	static GPIOPin solenoid1(GPIOA, 2, OUTPUT_MODE, PULL_DOWN);
//	static GPIOPin solenoid2(GPIOA, 3, OUTPUT_MODE, PULL_DOWN);
//
//	//TODO: Make usable by other modules or alternatively setup pins in other peripherals
//}

void Statemachine::setup()
{

	//Initialize and link Peripherals
	static Output led_pin(GPIOB, 1, NO_PULL);
	static LED led(&led_pin);
	peripherals.led = &led;

	static Input arm_button(GPIOF, 0, PULL_UP);
	peripherals.button = &arm_button;

	static Display display;
	peripherals.display = &display;
	peripherals.display->setup(&u8x8_byte_hw_i2c, &u8x8_stm32_gpio_and_delay);

	static Encoder encoder;
	peripherals.encoder = &encoder;
	peripherals.encoder->setup();

	static ADCobj adc;
	peripherals.adc = &adc;
	peripherals.adc->setup();

	static Menu menu(&encoder, &display);
	peripherals.menu = &menu;
	peripherals.menu->setup();

	setup_OC();

	//Construct States
	//static std::array<State*, 5> statearray;
	static State* statearray[5];
	states = statearray;

	//static std::vector<State*> statevector;

	//IDLE State
	static State idle_state;
	idle_state.id = statemachine::IDLE_STATE;

	idle_state.entry_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		char c[] = "Idle";
		(*peripherals).display->print_state(c);
		//(*peripherals).led->on();
		return;
	};

	idle_state.body_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		//check encoder
		if (((*peripherals).encoder->increment() != 0) || (*peripherals).encoder->button())
		{
			*active_state = statemachine::MENU_STATE;
			return;
		}

		//read voltage drop
//		(*peripherals).adc->read(TEST_CURRENT);

		uint8_t button_state = (*peripherals).button->read();

		//change the state to PRESSED_STATE when button is pressed (i.e. LOW) and electrode has contact
#ifdef DEBUG_3V3
		if ((button_state == BUTTON_PRESSED) && ((*peripherals).adc->vsense > 300))
#else
			if (((*peripherals).button->read() == BUTTON_PRESSED) && (*peripherals).adc->vsense > 1024)
#endif
			{
				*active_state = statemachine::PRESSED_STATE;
			}

		// HAL_Delay(10);
		return;
	};

	idle_state.exit_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		return;
	};

	//Add to vector
	statearray[idle_state.id] = &idle_state;
	//statevector.push_back(&idle_state);

	//PRESSED State

//	static State pressed_state;
//	pressed_state.id = statemachine::PRESSED_STATE;
//	//Configure actions
//	pressed_state.entry_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		char c[] = "Activation";
//		(*peripherals).display->print_state(c);
//		return;
//	};
//	pressed_state.body_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		if ((*peripherals).button->read() == BUTTON_RELEASED) //Wait for release (i.e. HIGH state) of ARM Button
//		{
//			(*peripherals).adc->read(TEST_CURRENT); //check for electrode contact
//#ifdef DEBUG_3V3
//			if (((*peripherals).adc->vsense) >= 300)
//#else
//				if (((*peripherals).adc->vsense) >= 1024)
//#endif
//				{ //Change to ARMED_STATE if still in contact
//					*active_state = statemachine::ARMED_STATE;
//				}
//				else
//				{ //Return to IDLE_STATE if no contact is found
//					*active_state = statemachine::IDLE_STATE;
//				}
//		}
//		return;
//	};
//	pressed_state.exit_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		HAL_Delay(50);
//		return;
//	};
//
//	//Add to vector
//	statearray[pressed_state.id] = &pressed_state;
//	//	statevector.push_back(&pressed_state);
//
//
//	//ARMED State
//	static State armed_state;
//	armed_state.id = statemachine::ARMED_STATE;
//	//Configure actions
//	armed_state.entry_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		char c[] = "ARMED";
//		(*peripherals).display->print_state(c);
//		(*peripherals).led->on();
//		(*peripherals).adc->vref = (*peripherals).adc->vsense;
//		return;
//	};
//	armed_state.body_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		(*peripherals).adc->read(TEST_CURRENT);
//		//check if the welder should be disarmed
//		if (((*peripherals).button->read()) == BUTTON_PRESSED)
//		{
//			*active_state = statemachine::PRESSED_STATE;
//			return;
//		}
//		else if ((((*peripherals).adc->vsense) * 4) <= ((*peripherals).adc->vref)){ //check for electrode to be lifted again
//			//fire pulse
//			weld_pulse(((*peripherals).menu->settings)[menu::PULSETIME_SETTING]);
//
//			//return to idle state
////			(*peripherals).display->print_state("FIRING");
//			*active_state = statemachine::IDLE_STATE;
//
//			//HAL_Delay(10);
//		}
//		return;
//	};
//	armed_state.exit_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
//		(*peripherals).led->off();
//		//TODO: Display Pulse properties: Energy, Pulsetime etc.
//		return;
//	};
//	//Add to vector
//	statearray[armed_state.id] = &armed_state;
//	//	statevector.push_back(&armed_state);
//

	//MENU State
	static State menu_state;
	menu_state.id = statemachine::MENU_STATE;
	//Configure actions
	menu_state.entry_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		(*peripherals).menu->enter();
		return;
	};
	menu_state.body_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		*active_state = statemachine::IDLE_STATE;
		return;
	};
	menu_state.exit_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		return;
	};
	//Add to vector
	statearray[menu_state.id] = &menu_state;
	//	statevector.push_back(&menu_state);


	//DEBUG State
	static State debug_state;
	debug_state.id = statemachine::DEBUG_STATE;
	//Configure actions
	debug_state.entry_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		return;
	};
	debug_state.body_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		return;
	};
	debug_state.exit_action = [](uint8_t *active_state, Peripherals *peripherals) -> void {
		return;
	};
	//Add to vector
	statearray[debug_state.id] = &debug_state;
	//	statevector.push_back(&debug_state);


	//	return &statevector;
	return;
}

void RCC_Interface::setup(){
	//TODO: Initialize the clocks for the interfaces I2C etc. or do it in their own setup, like with the GPIO...
	return;
}

void weld_pulse(setting_t *pulsetime_setting)
{
	//get the pulsetime from the pulsetime_setting struct:
	uint16_t pulsetime = (pulsetime_setting->setting_value) * (pulsetime_setting->scale);

	TIM14->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF); //clear update and capture/compare interrupt flags
	TIM14->CNT = 0; //clear counter

	//	TIM14->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos); //reset OC mode
	//	TIM14->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); //set pwm mode 1

	TIM14->CCR1 = pulsetime; //pulsetime in 0.1ms

	//IMPROVE: Start ADC voltage/current read

	TIM14->CR1 |= TIM_CR1_CEN; //Enable counter a.k.a. start timer

	while(!(((TIM14->SR) & TIM_SR_CC1IF)>0)){} //wait for timer to finish
	TIM14->CR1 &= ~TIM_CR1_CEN; //Stop timer

	//IMPROVE: Stop ADC voltage/current read
	//IMPROVE: Compute the integral over U*I and present the energy on screen

	//	TIM14->CCMR1 &= ~(0b111 << TIM_CCMR1_OC1M_Pos); //reset OC mode
	//	TIM14->CCMR1 |= (0b100 << TIM_CCMR1_OC1M_Pos); //force inactive mode

}

void setup_OC()
{	//Setup Output pin
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
}

uint8_t u8x8_stm32_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,
		U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,
		U8X8_UNUSED void *arg_ptr)
{
	uint8_t i;
	uint8_t buffer[4];
	switch (msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT: // called once during init phase of u8g2/u8x8
		break;                         // can be used to setup pins
	case U8X8_MSG_DELAY_NANO:          // delay arg_int * 1 nano second
		break;
	case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds

		for (i = 0; i < arg_int; i++)
		{
			__NOP();
		}
		break;
	case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
		for (i = 0; i < arg_int; i++)
		{
			for (uint16_t n = 0; n < 320; n++)
			{
				__NOP();
			}
		}

		break;
	case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
		//HAL_Delay(arg_int);
		break;
	case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz

		break;             // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
	case U8X8_MSG_GPIO_D0: // D0 or SPI clock pin: Output level in arg_int
		//case U8X8_MSG_GPIO_SPI_CLOCK:
		break;
	case U8X8_MSG_GPIO_D1: // D1 or SPI data pin: Output level in arg_int
		//case U8X8_MSG_GPIO_SPI_DATA:
		break;
	case U8X8_MSG_GPIO_D2: // D2 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D3: // D3 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D4: // D4 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D5: // D5 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D6: // D6 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_D7: // D7 pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_E: // E/WR pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_CS: // CS (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_DC: // DC (data/cmd, A0, register select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_RESET: // Reset pin: Output level in arg_int
		buffer[0] = 0x00;
		buffer[1] = 0x8d; //set charge pump
		buffer[2] = 0x10; //disable charge pump
		HAL_I2C_Master_Transmit(&hi2c1, 0x3c << 1, (uint8_t *)buffer, 3, 500);
		//HAL_Delay(200);
		buffer[2] = 0x14; //enable charge pump
		buffer[3] = 0xAF; //Display ON
		HAL_I2C_Master_Transmit(&hi2c1, 0x3c << 1, (uint8_t *)buffer, 4, 500);

		break;
	case U8X8_MSG_GPIO_CS1: // CS1 (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_CS2: // CS2 (chip select) pin: Output level in arg_int
		break;
	case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin
		break;                    // arg_int=1: Input dir with pullup high for I2C clock pin
	case U8X8_MSG_GPIO_I2C_DATA:  // arg_int=0: Output low at I2C data pin
		break;                    // arg_int=1: Input dir with pullup high for I2C data pin
	case U8X8_MSG_GPIO_MENU_SELECT:
		//		u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_NEXT:
		//		u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_PREV:
		//		u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
		break;
	case U8X8_MSG_GPIO_MENU_HOME:
		//		u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
		break;
	default:
		u8x8_SetGPIOResult(u8x8, 1); // default return value
		break;
	}
	return 1;
}

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	switch (msg)
	{
	case U8X8_MSG_BYTE_SEND:
		HAL_I2C_Master_Transmit(&hi2c1, 0x3c << 1, (uint8_t *)arg_ptr, arg_int, 500);
		break;
	case U8X8_MSG_BYTE_INIT:
		/* add your custom code to init i2c subsystem */
		break;
	case U8X8_MSG_BYTE_SET_DC:
		/* ignored for i2c */
		break;
	case U8X8_MSG_BYTE_START_TRANSFER:
		break;
	case U8X8_MSG_BYTE_END_TRANSFER:
		break;
	default:
		return 0;
	}
	return 1;
}
