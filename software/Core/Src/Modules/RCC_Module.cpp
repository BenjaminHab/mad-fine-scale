#include "RCC_Module.hpp"


RCC_Interface::RCC_Interface(){
	setup();
	return;
}

void RCC_Interface::set_system_clock(Clock_Source source_){

	//IMPROVE: Change to HSI

//	if(source_ == PLL){
//		//IMPROVE:
//		RCC->CR &= ~(RCC_CR_PLLON);//Disable PLL by resetting PLLON
//		while(((RCC->CR) & (RCC_CR_PLLRDY)) > 0){} //Wait until PLLRDY is cleared
//		RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN_Msk); //Reset Multiplier
//		RCC->PLLCFGR |=  (12 << RCC_PLLCFGR_PLLN_Pos); //Set multiplier to 12
//		RCC->CFGR2 &= ~(RCC_CFGR2_PREDIV); //Clear predivision a.k.a. no predivision
//		RCC->CR |=  (RCC_CR_PLLON);//Enable PLL by setting PLLON to 1
//		while(((RCC->CR) & (RCC_CR_PLLRDY)) == 0){} //Wait until PLLRDY is set
//		RCC->CFGR |= RCC_CFGR_SW_PLL;//select PLL as System Clock Source
//		while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){} //Wait for the change to take effect
//	}
	//IMPROVE: if source_ == HSE -> Change to HSE
	return;
}


