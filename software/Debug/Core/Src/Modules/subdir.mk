################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/Modules/ADC_Module.cpp \
../Core/Src/Modules/Display_Module.cpp \
../Core/Src/Modules/Encoder_Module.cpp \
../Core/Src/Modules/GPIO_Module.cpp \
../Core/Src/Modules/Init_Modules.cpp \
../Core/Src/Modules/LED_Module.cpp \
../Core/Src/Modules/Menu_Module.cpp \
../Core/Src/Modules/RCC_Module.cpp \
../Core/Src/Modules/Statemachine_Module.cpp \
../Core/Src/Modules/Test_Module.cpp 

OBJS += \
./Core/Src/Modules/ADC_Module.o \
./Core/Src/Modules/Display_Module.o \
./Core/Src/Modules/Encoder_Module.o \
./Core/Src/Modules/GPIO_Module.o \
./Core/Src/Modules/Init_Modules.o \
./Core/Src/Modules/LED_Module.o \
./Core/Src/Modules/Menu_Module.o \
./Core/Src/Modules/RCC_Module.o \
./Core/Src/Modules/Statemachine_Module.o \
./Core/Src/Modules/Test_Module.o 

CPP_DEPS += \
./Core/Src/Modules/ADC_Module.d \
./Core/Src/Modules/Display_Module.d \
./Core/Src/Modules/Encoder_Module.d \
./Core/Src/Modules/GPIO_Module.d \
./Core/Src/Modules/Init_Modules.d \
./Core/Src/Modules/LED_Module.d \
./Core/Src/Modules/Menu_Module.d \
./Core/Src/Modules/RCC_Module.d \
./Core/Src/Modules/Statemachine_Module.d \
./Core/Src/Modules/Test_Module.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Modules/%.o: ../Core/Src/Modules/%.cpp Core/Src/Modules/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0plus -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G070xx -c -I../Core/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Modules

clean-Core-2f-Src-2f-Modules:
	-$(RM) ./Core/Src/Modules/ADC_Module.d ./Core/Src/Modules/ADC_Module.o ./Core/Src/Modules/Display_Module.d ./Core/Src/Modules/Display_Module.o ./Core/Src/Modules/Encoder_Module.d ./Core/Src/Modules/Encoder_Module.o ./Core/Src/Modules/GPIO_Module.d ./Core/Src/Modules/GPIO_Module.o ./Core/Src/Modules/Init_Modules.d ./Core/Src/Modules/Init_Modules.o ./Core/Src/Modules/LED_Module.d ./Core/Src/Modules/LED_Module.o ./Core/Src/Modules/Menu_Module.d ./Core/Src/Modules/Menu_Module.o ./Core/Src/Modules/RCC_Module.d ./Core/Src/Modules/RCC_Module.o ./Core/Src/Modules/Statemachine_Module.d ./Core/Src/Modules/Statemachine_Module.o ./Core/Src/Modules/Test_Module.d ./Core/Src/Modules/Test_Module.o

.PHONY: clean-Core-2f-Src-2f-Modules

