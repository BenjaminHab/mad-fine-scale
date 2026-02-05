//! adc oversampling example
//!
//! This example uses adc oversampling to achieve 16bit data

#![no_std]
#![no_main]

use cortex_m::delay;
use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::adc::{Adc, SampleTime};
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_time::Timer;
use embassy_stm32::Config;

use embassy_stm32::time::Hertz;

use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    let mut config = Config::default();
    {
        use embassy_stm32::rcc::*;
        config.rcc.hse = Some(Hse {
            freq: Hertz(48_000_000),
            mode: HseMode::Oscillator,
        });

        config.rcc.sys = Sysclk::HSE; // 48 Mhz
        config.rcc.ahb_pre = AHBPrescaler::DIV1; // 300 Mhz
        config.rcc.apb1_pre = APBPrescaler::DIV1; // 150 Mhz
    }
    let p = embassy_stm32::init(config);
    info!("Adc oversample test");

    let mut ina_enable = Output::new(p.PA15, Level::Low, Speed::Low);
    let mut bridge_enable = Output::new(p.PA3, Level::Low, Speed::Low); // P-channel MOSFET -> Pull low to enable

    // Wait a little, so the INA gets the message to self-calibrate
    Timer::after_millis(100).await;

    // Switch on Instrumentation amplifier and bridge power supply
    let _ = ina_enable.set_high();
    let _ = bridge_enable.set_low();

    let mut adc = Adc::new(p.ADC1);
    adc.set_sample_time(SampleTime::CYCLES79_5);
    let mut v_pin = p.PA0;

    // From https://www.st.com/resource/en/reference_manual/rm0444-stm32g0x1-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
    // page373 15.8 Oversampler
    // Table 76. Maximum output results vs N and M. Grayed values indicates truncation
    // 0x00 oversampling ratio X2
    // 0x01 oversampling ratio X4
    // 0x02 oversampling ratio X8
    // 0x03 oversampling ratio X16
    // 0x04 oversampling ratio X32
    // 0x05 oversampling ratio X64
    // 0x06 oversampling ratio X128
    // 0x07 oversampling ratio X256
    adc.set_oversampling_ratio(0x03);
    adc.set_oversampling_shift(0b0000);
    adc.oversampling_enable(true);

    let mut v_ref = adc.enable_vrefint();
    let ref_bits = adc.blocking_read(&mut v_ref);
    Timer::after_millis(200).await;
    let mut v_bat = adc.enable_vbat();

    VREFINT_CAL
    

    loop {
        // let ref_bits = adc.blocking_read(&mut v_ref);
        let bat_bits = adc.blocking_read(&mut v_bat);
        let v = convert_to_millivolts(ref_bits, bat_bits);
        info!("vbat: {}, ref_bits: {}, bat_bits: {}", 3*v, ref_bits, bat_bits); //max 65520 = 0xFFF0
        Timer::after_millis(100);
        let ina_ref_bits = adc.blocking_read(&mut v_pin);
        let v = convert_to_millivolts(ref_bits, bat_bits);
        info!("vref_ina: {}, ref_bits: {}, vref_ina_bits: {}", v, ref_bits, ina_ref_bits); //max 65520 = 0xFFF0
        Timer::after_millis(1000).await;
    }
}


fn convert_to_millivolts(vref_bits: u16, bits: u16) -> u16{
    // Embedded internal reference voltage should be 1212mV for STM32G0
    const VREFINT_MV: u32 = 1212; // mV

    (u32::from(bits) * VREFINT_MV / u32::from(vref_bits)) as u16
}