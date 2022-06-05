#include <main.h>
#include "ADC_Module.hpp"
#include "adc.h"

ADCobj::ADCobj()
{
    return;
}

uint32_t ADCobj::read(uint8_t channel)
{
    vsense = 0;

    //Select Channel for conversion:
    ADC1->CHSELR = (1 << channel);

    //Set single conversion mode:
    ADC1->CFGR1 &= ~(ADC_CFGR1_CONT_Msk);

    //Reset End Of Conversion flag (should be unnecessary)
    //ADC1->ISR |= ADC_ISR_EOC;

    /* Performs the AD conversion */
    ADC1->CR |= ADC_CR_ADSTART; /* start the ADC conversion */
    while ((ADC1->ISR & ADC_ISR_EOC) == 0){}; /* wait end of conversion */

    vsense = ADC1->DR;

    return vsense;
}

//TODO: Add a DMA Conversion mode to measure both channels during a weld pulse
//TODO: Add a method to convert the measurement to volts (See reference manual, page 205)
