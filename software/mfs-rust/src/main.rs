#![no_std]
#![no_main]

// Imports
extern crate cortex_m;
extern crate cortex_m_rt as rt;
extern crate panic_semihosting;
extern crate stm32g0xx_hal as hal;
extern crate fugit;

//use cortex_m_semihosting::hprintln;
use hal::analog::adc::{OversamplingRatio, Precision, SampleTime, VBat};
use hal::prelude::*;
use hal::rcc::Config;
use hal::stm32;
use rt::entry;

use rtt_target::{rprintln, rtt_init_print};

use fugit::Rate;

// TODO: Get list with Pin aliases
// TODO: Get 48MHz HSE Oscillator working (and check it)
// TODO: Create remote repository and sync
// TODO: Get I2C for Rheo working
// TODO: Get I2C for Display working
// TODO: Get INA calibration working
// TODO: Get peripheral tests working?
// TODO: Get button input etc. working
// TODO: Low power modes & switch off and reinit peripherals
// TODO: Watchdog?
// TODO: Organize into library?


#[entry]
fn main() -> ! {
    // Init rtt print function for debugging
    rtt_init_print!();

    // Setup handles for device peripherals
    let dp = stm32::Peripherals::take().expect("cannot take peripherals");
    
    // Setup delay
    let clock_config = Config::new(hal::rcc::SysClockSrc::HSE(48.MHz()));
    let mut rcc = dp.RCC.freeze(clock_config);
    let mut delay = dp.TIM15.delay(&mut rcc);

    // GPIO setup
    let gpioa = dp.GPIOA.split(&mut rcc);
    let gpiob = dp.GPIOB.split(&mut rcc);

    let mut led = gpiob.pb4.into_push_pull_output();
    let mut bridge_enable = gpioa.pa3.into_push_pull_output(); // P-channel MOSFET -> Pull low to enable
    //let mut display_enable = gpioa.pa2.into_push_pull_output(); // P-channel MOSFET -> Pull low to enable
    let mut ina_enable = gpioa.pa15.into_push_pull_output();
    
    // ADC setup
    let mut adc = dp.ADC.constrain(&mut rcc);
    adc.set_sample_time(SampleTime::T_80);
    adc.set_precision(Precision::B_12);
    adc.set_oversampling_ratio(OversamplingRatio::X_16);
    adc.set_oversampling_shift(16);
    adc.oversampling_enable(true);

    delay.delay(20.micros());
    adc.calibrate();

    let mut adc_pin = gpioa.pa1.into_analog();

    let mut vbat = VBat::new();
    vbat.enable(&mut adc);

    // Switch on Instrumentation amplifier and bridge power supply
    ina_enable.set_high();
    bridge_enable.set_low();

    // Encoder setup
    let switch = gpioa.pa0.into_pull_up_input();
    let qei = dp.TIM3.qei((gpioa.pa6, gpioa.pa7), &mut rcc);

    loop {
        //led.toggle().unwrap();
        //delay.delay(500.millis());
        let count = qei.count();
        if switch.is_low().unwrap() {
            //hprintln!("Counter: {}", count);
            rprintln!("Counter: {}", count);
            let u_mv = adc.read_voltage(&mut adc_pin).expect("adc read failed");
            let u_bat = adc.read_voltage(&mut vbat).expect("adc read failed");
            rprintln!("VBat: {}mV | Bridge: {}mV", u_bat * 3, u_mv);
            led.set_high().unwrap();
            delay.delay(500.millis());
            led.set_low().unwrap();
        }
    }
}