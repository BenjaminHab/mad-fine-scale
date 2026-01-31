#![no_std]
#![no_main]

use defmt::*;
use defmt_rtt as _;
use embassy_executor::Spawner;
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::{bind_interrupts, i2c, peripherals, time::Hertz};
use embassy_time::Timer;
use panic_probe as _;
use ssd1306::{prelude::*, I2CDisplayInterface, Ssd1306Async};

bind_interrupts!(struct Irqs {
    I2C1 => i2c::EventInterruptHandler<peripherals::I2C1>, i2c::ErrorInterruptHandler<peripherals::I2C1>;
});

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());

    // Switch PA2 low to enable display
    let mut display_enable = Output::new(p.PA3, Level::Low, Speed::Low);
    let _ = display_enable.set_low();

    let mut i2c_conf: i2c::Config = i2c::Config::default();
    i2c_conf.sda_pullup = true;
    i2c_conf.scl_pullup = true;
    i2c_conf.timeout = embassy_time::Duration::from_millis(1000);

    // let mut i2c = embassy_stm32::i2c::I2c::new(
    let i2c = embassy_stm32::i2c::I2c::new(
        p.I2C1,
        p.PA9,
        p.PA10,
        Irqs,
        p.DMA1_CH1,
        p.DMA1_CH2,
        Hertz(40_000),
        i2c_conf,
    );

    let interface = I2CDisplayInterface::new(i2c);
    let mut display = Ssd1306Async::new(interface, DisplaySize128x32, DisplayRotation::Rotate0)
        .into_terminal_mode();
    let _init_result = display.init().await;
    let _ = display.set_brightness(Brightness::DIM).await;

    loop {
        Timer::after_millis(1000).await;
        let _init_result = display.init().await;
        let _ = display.set_brightness(Brightness::DIM).await;
        match display.clear().await {
            Ok(_) => {
                debug!("Clear display successful");
            }
            Err(_err) => {
                error!("Clear display unsuccesful");
            }
        }

        //Display Hello Rust
        Timer::after_millis(1000).await;
        match display.write_str("Hello Rust!").await {
            Ok(_) => {
                debug!("Write display successful");
            }
            Err(_err) => {
                error!("Write display unsuccesful");
            }
        }
    }

    // test i2c with external device on breadboard

    // Acc addresses: 0x19 & 0x1E & 0x6B

    // loop{
    //     let mut rx_buf: [u8; 1] = [0x00];
    //     let tx_buf: [u8; 1] = [0x0F];
    //     match i2c
    //     .write_read(0x6B, &tx_buf, &mut rx_buf)
    //     .await
    //     {
    //         Ok(_) => {
    //             debug!("Read {:?} from wiper0", rx_buf[0]);
    //         }
    //         Err(err) => {
    //             error!("error: {:?}", err);
    //         }
    //     }
    //     Timer::after_millis(1000).await;
    // }
}
