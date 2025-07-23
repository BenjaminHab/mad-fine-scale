#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::adc::{Adc, SampleTime};
use embassy_time::Timer;
use embassy_stm32::time::Hertz;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::Config;
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

    info!("Hello World!");

    let mut led = Output::new(p.PB4, Level::High, Speed::Low);



    // let mut adc = Adc::new(p.ADC1);
    // adc.set_sample_time(SampleTime::CYCLES79_5);
    // let mut pin = p.PA1;

    // let mut vrefint = adc.enable_vrefint();
    // let vrefint_sample = adc.blocking_read(&mut vrefint);
    // let convert_to_millivolts = |sample| {
    //     // From https://www.st.com/resource/en/datasheet/stm32g031g8.pdf
    //     // 6.3.3 Embedded internal reference voltage
    //     const VREFINT_MV: u32 = 1212; // mV

    //     (u32::from(sample) * VREFINT_MV / u32::from(vrefint_sample)) as u16
    // };

    // loop {
    //     let v = adc.blocking_read(&mut pin);
    //     info!("--> {} - {} mV", v, convert_to_millivolts(v));
    //     debug!("In the loop!");
    //     Timer::after_millis(100).await;
    // }

    loop {
        info!("high");
        led.set_high();
        Timer::after_millis(300).await;

        info!("low");
        led.set_low();
        Timer::after_millis(300).await;
    }
}
