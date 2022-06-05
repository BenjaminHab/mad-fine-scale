//#include "Modules.hpp"
#include <main.h>
#include "GPIO_Module.hpp"

GPIOPin::GPIOPin(){
	return;
}

GPIOPin::GPIOPin(GPIO_TypeDef* gpio_, uint16_t pin_, PinMode mode_, PullMode pullmode_){
	gpio = gpio_;
	pin = pin_;
	set_peripheral_clock();
	set_mode(mode_);
	set_pull(pullmode_);
	return;
}

void GPIOPin::set_peripheral_clock(){
	//Enable peripheral clock
	if(gpio == GPIOA){
		RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	}else if(gpio == GPIOB){
		RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
	}else if(gpio == GPIOC){
		RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
	}else if(gpio == GPIOD){
		RCC->IOPENR |= RCC_IOPENR_GPIODEN;
#ifdef GPIOE
	}else if(gpio == GPIOE){
		RCC->IOPENR |= RCC_IOPENR_GPIOEEN;
#endif
	}else if(gpio == GPIOF){
		RCC->IOPENR |= RCC_IOPENR_GPIOFEN;
	}
	return;
}

void GPIOPin::set_mode(PinMode mode_=ANALOG_MODE){
	gpio->MODER &= ~(0b11<<(2*pin)); //Reset Pin mode
	gpio->MODER |=  (mode_<<(2*pin)); //Set Pin mode
	return;
}

void GPIOPin::set_pull(PullMode pull_mode_=NO_PULL){
	gpio->PUPDR &= ~(0b11<<(2*pin)); //Reset pull mode
	gpio->PUPDR |=  (pull_mode_<<(2*pin)); //Set pull mode
	return;
}

void GPIOPin::set_af(AlternateFunction AF_){
	uint8_t i = pin > 7;
	gpio->AFR[i] &= ~(0b1111 << (4*pin));
	gpio->AFR[i] |=  (   AF_ << (4*pin));
	return;
}

void GPIOPin::set_speed(OutputSpeed speed_){
	gpio->OSPEEDR &= ~(0b11 << pin*2);
	gpio->OSPEEDR |= (speed_ << pin*2);
	return;
}

Input::Input(){
	return;
}

Input::Input(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_){
	gpio = gpio_;
	pin = pin_;
	set_peripheral_clock();
	set_mode(INPUT_MODE);
	set_pull(pullmode_);
}

uint8_t Input::read(){
	return (((gpio->IDR) & (1<<pin))>0);
}

Output::Output(){
	return;
}

Output::Output(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_){
	gpio = gpio_;
	pin = pin_;
	set_peripheral_clock();
	set_mode(OUTPUT_MODE);
	set_pull(pullmode_);
}

void Output::set(PinLevel level_ /*=HIGH*/){
	if(level_ == LOW){
		gpio->BSRR |= (1<<(pin+16));
	}else{
		gpio->BSRR |= (1<<(pin));
	}
	return;
}

void Output::reset(){
	gpio->BSRR |= (1<<(pin+16));
	return;
}

uint8_t Output::read(){
	return (((gpio->ODR) & (1<<pin))>0);
}

AFIO::AFIO(){
	return;
}

AFIO::AFIO(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_, AlternateFunction AF_){
	gpio = gpio_;
	pin = pin_;
	set_peripheral_clock();
	set_mode(ALTERNATE_MODE);
	set_pull(pullmode_);
	set_af(AF_);
}
