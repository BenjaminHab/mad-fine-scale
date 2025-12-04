#![no_std]
#![no_main]

// Imports
extern crate cortex_m;
extern crate cortex_m_rt as rt;
extern crate fugit;
extern crate panic_semihosting;
extern crate stm32g0xx_hal as hal;

//use cortex_m_semihosting::hprintln;
use hal::analog::adc::{OversamplingRatio, Precision, SampleTime, VBat};
use hal::hal::blocking::serial::write;
use hal::i2c;
use hal::prelude::*;
use hal::rcc::Config;
use hal::stm32;
use rt::entry;

use rtt_target::{rprintln, rtt_init_print};

use fugit::Rate;

// TODO: Get list with Pin aliases
// TODO: Get 48MHz HSE Oscillator working (and check it)
// TODO: Get I2C for Rheo working
// TODO: Get I2C for Display working
// TODO: Get INA calibration working
// TODO: Get button input etc. working
// TODO: Watchdog?

// General:
// TODO: Test library for peripheral tests?
// TODO: State machine library
// TODO: Organize into mad fine scale library?

// Power saving:
// TODO: Low power modes, switch to lsi oscillator clock?
// TODO: Use standby mode if not in use
// TODO: switch off and reinit peripherals
// TODO: Switch non-needed pins to analog in general?
// TODO: Do something with the rheo on standby entry?
// TODO: Switch all pins to analog (that includes pullup/pulldown right?) before standby mode entry
//       Exceptions: Encoder button/WKUP1, INA/Display/bridge enable pins (pullup, then analog)
// TODO: Wakeup via WKUP1 pin, a.k.a. the encoder button
// TODO: Switch off condition: 2min without input (?) and 5s hold on encoder button

// Display:
// ssd1306 and/or embedded-graphics crates look promising

#[entry]
fn main() -> ! {
    // Init rtt print function for debugging
    rtt_init_print!();
    rprintln!("Begin setup");
    // Setup handles for device peripherals
    let dp = stm32::Peripherals::take().expect("cannot take peripherals");

    // Setup delay
    let clock_config = Config::new(hal::rcc::SysClockSrc::HSE(48.MHz()));
    let mut rcc = dp.RCC.freeze(clock_config);
    let mut delay = dp.TIM15.delay(&mut rcc);

    // GPIO setup
    // enable peripheral clocks
    let gpioa = dp.GPIOA.split(&mut rcc);
    let gpiob = dp.GPIOB.split(&mut rcc);

    // setup general pins
    let mut led = gpiob.pb4.into_push_pull_output();
    let mut bridge_enable = gpioa.pa3.into_push_pull_output(); // P-channel MOSFET -> Pull low to enable
    let mut display_enable = gpioa.pa2.into_push_pull_output(); // P-channel MOSFET -> Pull low to enable
    let mut ina_enable = gpioa.pa15.into_push_pull_output();
    let mut rheo_hvc = gpioa.pa4.into_push_pull_output(); // rheo high voltage command

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

    // Encoder setup
    let switch = gpioa.pa0.into_pull_up_input();
    let qei = dp.TIM3.qei((gpioa.pa6, gpioa.pa7), &mut rcc);
    // TODO: Set counter to medium value to prevent over/underflow

    // I2C setup
    let sda = gpioa.pa10.into_open_drain_output_in_state(PinState::High);
    let scl = gpioa.pa9.into_open_drain_output_in_state(PinState::High);

    let rheo_addr: u8 = 0b0101111;
    let rheo_read: u8 = 0b11;
    let rheo_write: u8 = 0b00;
    let rheo_increment: u8 = 0b01;
    let rheo_decrement: u8 = 0b10;
    let rheo_volatile_wiper0_addr: u8 = 0x00;
    let rheo_nonvolatile_wiper0_addr: u8 = 0x02;

    let mut i2c = dp
        .I2C1
        .i2c(sda, scl, i2c::Config::with_timing(0x2030_3e5d), &mut rcc);

    let read_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_read << 2]; //
    let increment_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_increment << 2];
    let mut write_wiper0: [u8; 2] = [rheo_volatile_wiper0_addr << 4 | rheo_write << 2, 0x00];
    let mut rx_buf: [u8; 2] = [0x00,0x00];

    // Switch on Instrumentation amplifier and bridge power supply
    let _ = ina_enable.set_high();
    let _ = bridge_enable.set_low();
    rprintln!("Begin loop");
    loop {
        //led.toggle().unwrap();
        delay.delay(500.millis());
        let count = qei.count();
        // if switch.is_low().unwrap() {
        rprintln!("Counter: {}", count);
        let u_mv = adc.read_voltage(&mut adc_pin).expect("adc read failed");
        let u_bat = adc.read_voltage(&mut vbat).expect("adc read failed");
        rprintln!("VBat: {}mV | Bridge: {}mV", u_bat * 3, u_mv);
        led.set_high().unwrap();
        delay.delay(500.millis());
        led.set_low().unwrap();
        // }

        // read wiper0
        match i2c.write_read(rheo_addr, &read_wiper0, &mut rx_buf) {
            Ok(_) => rprintln!("Read {:?} from wiper0", rx_buf),
            Err(err) => rprintln!("error: {:?}", err),
        }

        delay.delay(100.millis());

        // write wiper0
        write_wiper0[1] = 64; // set data byte
        match i2c.write(rheo_addr, &write_wiper0) {
            Ok(_) => rprintln!("Wrote {:?} to wiper0", &write_wiper0[1]),
            Err(err) => rprintln!("error: {:?}", err),
        }

        delay.delay(100.millis());

        // read wiper0
        match i2c.write_read(rheo_addr, &read_wiper0, &mut rx_buf) {
            Ok(_) => rprintln!("Read {:?} from wiper0", rx_buf[1]),
            Err(err) => rprintln!("error: {:?}", err),
        }

        delay.delay(100.millis());

        // increment wiper0 5 times
        for i in 0..5 {
            match i2c.write(rheo_addr, &increment_wiper0) {
                Ok(_) => rprintln!("Incremented wiper0"),
                Err(err) => rprintln!("error: {:?}", err),
            }
            delay.delay(100.millis());
        }

        delay.delay(100.millis());

        // read wiper0
        match i2c.write_read(rheo_addr, &read_wiper0, &mut rx_buf) {
            Ok(_) => rprintln!("Read {:?} from wiper0", rx_buf[1]),
            Err(err) => rprintln!("error: {:?}", err),
        }

        // match i2c.write(rheo_addr, &[tx_buf]) {
        //     Ok(_) => rprintln!("ok"),
        //     Err(err) => rprintln!("error: {:?}", err),
        // }

        // TODO: Try increment and decrement and read if values have changed

        // match i2c.write_read(rheo_addr, &tx_buf, &mut rx_buf) {
        // match i2c.read(rheo_addr, &mut rx_buf) {
        //     Ok(_) => rprintln!("ok, {:?}", rx_buf),
        //     Err(err) => rprintln!("error: {:?}", err),
        // }
    }
}
