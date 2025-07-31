#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::adc::{Adc, SampleTime};
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::i2c::{self, I2c};
use embassy_stm32::time::Hertz;
use embassy_stm32::Config;
use embassy_stm32::{bind_interrupts, peripherals};
use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

bind_interrupts!(struct Irqs {
    I2C1 => i2c::EventInterruptHandler<peripherals::I2C1>, i2c::ErrorInterruptHandler<peripherals::I2C1>;
});

const rheo_addr: u8 = 0b0101111;
const rheo_read: u8 = 0b1100;
const rheo_write: u8 = 0b0000;
const rheo_increment: u8 = 0b0100;
const rheo_decrement: u8 = 0b1000;
const rheo_volatile_wiper0_addr: u8 = 0x00;
const rheo_nonvolatile_wiper0_addr: u8 = 0x02;
const read_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_read << 2]; //
const increment_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_increment << 2];

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
    debug!("Base setup complete!");

    let mut led = Output::new(p.PB4, Level::High, Speed::Low);
    let mut ina_enable = Output::new(p.PA15, Level::High, Speed::Low); 
    let mut bridge_enable = Output::new(p.PA3, Level::Low, Speed::Low); // P-channel MOSFET -> Pull low to enable


        // Switch on Instrumentation amplifier and bridge power supply
        // let _ = ina_enable.set_high();
        // let _ = bridge_enable.set_low();
    debug!("GPIO setup complete!");

    let mut adc = Adc::new(p.ADC1);
    adc.set_sample_time(SampleTime::CYCLES79_5);
    let mut pin = p.PA1;
    let mut vrefint = adc.enable_vrefint();
    let vrefint_sample = adc.blocking_read(&mut vrefint);
    let convert_to_millivolts = |sample| {
        // From stm32g0x1 datasheet 17.2: Reference voltage is 2.048V
        const VREFINT_MV: u32 = 2048; // mV

        (u32::from(sample) * VREFINT_MV / u32::from(vrefint_sample)) as u16
    };
    // CHECK: Possibly check the VREFBUF config to make sure the internal reference voltage is used
    debug!("ADC setup complete!");

    let mut i2c_conf: i2c::Config = i2c::Config::default();
    i2c_conf.sda_pullup = true;
    i2c_conf.scl_pullup = true;
    i2c_conf.timeout = embassy_time::Duration::from_millis(1000);

    let mut i2c = I2c::new(
        p.I2C1,
        p.PA9,
        p.PA10,
        Irqs,
        p.DMA1_CH1,
        p.DMA1_CH2,
        Hertz(100_000),
        i2c_conf,
    );

    let mut write_wiper0: [u8; 2] = [rheo_volatile_wiper0_addr << 4 | rheo_write << 2, 0x00];
    let mut rx_buf: [u8; 2] = [0x00, 0x00];
    debug!("I2C setup complete!");

    debug!("Entering loop!");
    loop {
        for i in 0..15 {
            // Read rheo
            match i2c.write_read(rheo_addr, &read_wiper0, &mut rx_buf).await {
                Ok(_) => debug!("Read {:?} from wiper0", rx_buf[1]),
                Err(err) => debug!("error: {:?}", err),
            }
            Timer::after_millis(100).await;

            // Write rheo
            write_wiper0[1] = 4 * i; // set data byte
            match i2c.write(rheo_addr, &write_wiper0).await {
                Ok(_) => debug!("Wrote {:?} to wiper0", &write_wiper0[1]),
                Err(err) => debug!("error: {:?}", err),
            }
            Timer::after_millis(100).await;

            // increment wiper0 5 times
            for i in 0..5 {
                match i2c.write(rheo_addr, &increment_wiper0).await {
                    Ok(_) => debug!("Incremented wiper0"),
                    Err(err) => debug!("error: {:?}", err),
                }
                Timer::after_millis(100).await;
            }

            Timer::after_millis(100).await;

            // read wiper0
            match i2c.write_read(rheo_addr, &read_wiper0, &mut rx_buf).await {
                Ok(_) => debug!("Read {:?} from wiper0", rx_buf[1]),
                Err(err) => debug!("error: {:?}", err),
            }
            Timer::after_millis(100).await;

            // Read ADC value
            let v = adc.blocking_read(&mut pin);
            info!("--> {} - {} mV", v, convert_to_millivolts(v));
            led.toggle();
            Timer::after_millis(1000).await;
        }
    }
}
