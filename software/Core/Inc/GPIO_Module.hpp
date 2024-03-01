#ifndef GPIO_Module_hpp
#define GPIO_Module_hpp

#include <main.h>

enum PinMode{
	INPUT_MODE=0b00,
	OUTPUT_MODE=0b01,
	ALTERNATE_MODE=0b10,
	ANALOG_MODE=0b11,
	NO_INPUT=0xff
};

enum PullMode{
	NO_PULL = 0b00,
	PULL_UP = 0b01,
	PULL_DOWN = 0b10
};

enum PinLevel{
	LOW = 0,
	HIGH = 1
};

enum ButtonState{
	BUTTON_PRESSED = 0,
	BUTTON_RELEASED = 1
};

enum AlternateFunction{
	AF0,
	AF1,
	AF2,
	AF3,
	AF4,
	AF5,
	AF6,
	AF7
};

enum OutputSpeed{
	LOW_SPEED = 0b00,
	MEDIUM_SPEED = 0b01,
	HIGH_SPEED = 0b11
};



class GPIOPin
{
public:
	GPIOPin();
	GPIOPin(GPIO_TypeDef* gpio_, uint16_t pin_, PinMode mode_, PullMode pullmode_);

	void set_peripheral_clock();
	void set_pull(PullMode pullmode_);
	void set_mode(PinMode mode_);
	void set_af(AlternateFunction AF_);
	void set_speed(OutputSpeed speed_);

	uint16_t pin;
	GPIO_TypeDef* gpio;
};

class Input : public GPIOPin{
public:
	//	using GPIOPin::GPIOPin;
	Input();
	Input(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_ = NO_PULL);
	uint8_t read();
};

class Output : public GPIOPin{
public:
	//	using GPIOPin::GPIOPin;
	Output();
	Output(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_ = NO_PULL);
	void set(PinLevel level_ = HIGH);
	void reset();
	uint8_t read();
};

class AFIO : public GPIOPin{
public:
//	using GPIOPin::GPIOPin;
	AFIO();
	AFIO(GPIO_TypeDef* gpio_, uint16_t pin_, PullMode pullmode_, AlternateFunction AF_);
};

#endif
