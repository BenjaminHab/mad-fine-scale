
#ifndef ADC_MODULE
#define ADC_MODULE

#include <main.h>
#include "adc.h"

//TODO: Define channels
#define TEMP_CHANNEL ADC_CHANNEL_0
#define CURRENT_CHANNEL ADC_CHANNEL_1
#define VOLTAGE_CHANNEL ADC_CHANNEL_3
//#define TEST_CURRENT_EXPECTED_GAIN_INV 9.25F
//#define WELD_CURRENT_EXPECTED_GAIN 71.213F

//Classes

class ADCobj
{
public:
    //Constructors
    ADCobj();

    //Setup function to be defined in user code:
    void setup();

    uint32_t read(uint8_t channel);

    uint32_t vsense;
    uint32_t vref;

private:
};

#endif
