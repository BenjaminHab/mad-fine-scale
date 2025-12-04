#![no_std]
#![no_main]

use defmt_rtt as _;
use embassy_executor::Spawner;
use embassy_stm32::{bind_interrupts, i2c, peripherals, time::Hertz};
use panic_probe as _;
use ssd1306::{prelude::*, I2CDisplayInterface, Ssd1306Async};

bind_interrupts!(struct Irqs {
    I2C1 => i2c::EventInterruptHandler<peripherals::I2C1>, i2c::ErrorInterruptHandler<peripherals::I2C1>;
});

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    
    let mut i2c_conf: i2c::Config = i2c::Config::default();
    i2c_conf.sda_pullup = true;
    i2c_conf.scl_pullup = true;
    i2c_conf.timeout = embassy_time::Duration::from_millis(1000);

    let i2c = embassy_stm32::i2c::I2c::new(
        p.I2C1,
        p.PA9,
        p.PA10,
        Irqs,
        p.DMA1_CH1,
        p.DMA1_CH2,
        Hertz(100_000),
        i2c_conf,
    );
    // let i2c = embassy_stm32::i2c::I2c::new(p.I2C1,p.PA9,p.PA10,Irqs,p.DMA1_CH1,p.DMA1_CH2,Hertz::khz(100),Default::default(),);
    let interface = I2CDisplayInterface::new(i2c);
    let mut display = Ssd1306Async::new(interface, DisplaySize128x64, DisplayRotation::Rotate0)
        .into_terminal_mode();
    display.init().await.unwrap();
    let _ = display.clear().await;
    //Display Hello Rust
    let _ = display.write_str("Hello Rust!").await;
    loop {

    }
}