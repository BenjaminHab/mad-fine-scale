#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::adc::{Adc, SampleTime};
use embassy_stm32::gpio::{Level, Output, Speed};
use embassy_stm32::i2c::{self, I2c};
use embassy_stm32::mode::Async;
use embassy_stm32::time::Hertz;
use embassy_stm32::Config;
use embassy_stm32::{bind_interrupts, peripherals};
use embassy_time::Timer;
// use embedded_hal_1::i2c::I2c as _;
use embedded_hal_async::i2c::I2c as _;
use {defmt_rtt as _, panic_probe as _};

use core::cell::RefCell;
use embassy_embedded_hal::shared_bus::asynch::i2c::I2cDevice;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_sync::blocking_mutex::NoopMutex;
use embassy_sync::mutex;
// use embedded_hal_1::i2c::I2c as _;
use static_cell::StaticCell;

use embassy_stm32::pac::flash::Flash; // Maybe this allows for reading the VRefint factory calibration values at 0x1FFF 75AA - 0x1FFF 75AB

use mfs_rust::rheo::Rheo;

bind_interrupts!(struct Irqs {
    I2C1 => i2c::EventInterruptHandler<peripherals::I2C1>, i2c::ErrorInterruptHandler<peripherals::I2C1>;
});

const rheo_addr: u8 = 0b0101111;
const rheo_read: u8 = 0b11;
const rheo_write: u8 = 0b00;
const rheo_increment: u8 = 0b01;
const rheo_decrement: u8 = 0b10;
const rheo_volatile_wiper0_addr: u8 = 0x00;
const rheo_nonvolatile_wiper0_addr: u8 = 0x02;
const read_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_read << 2]; //
const increment_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_increment << 2];
const decrement_wiper0: [u8; 1] = [rheo_volatile_wiper0_addr << 4 | rheo_decrement << 2];

// static I2C_BUS: StaticCell<NoopMutex<RefCell<I2c<'static, Async, i2c::Master>>>> =
//     StaticCell::new();
type I2cBus = mutex::Mutex<NoopRawMutex, I2c<'static, Async, i2c::Master>>;

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
    let mut ina_enable = Output::new(p.PA15, Level::Low, Speed::Low);
    let mut bridge_enable = Output::new(p.PA3, Level::Low, Speed::Low); // P-channel MOSFET -> Pull low to enable

    // Wait a little, so the INA gets the message to self-calibrate
    Timer::after_millis(100).await;

    // Switch on Instrumentation amplifier and bridge power supply
    let _ = ina_enable.set_high();
    let _ = bridge_enable.set_low();
    debug!("GPIO setup complete!");

    // Raw read from memory
    const ADC: *mut u16 = (0x40012400) as *mut u16;
    const ADC_DR: *mut u16 = ADC.wrapping_add(0x40);
    const VREFINT: *mut u16 = (0x1FFF75AA) as *mut u16;
    unsafe {
        info!("VREFINT_LOW {}", core::ptr::read_volatile(VREFINT));
        info!("ADC_DR {}", core::ptr::read_volatile(ADC_DR));
    }

    let mut adc = Adc::new(p.ADC1);
    adc.set_sample_time(SampleTime::CYCLES1_5);
    adc.set_oversampling_ratio(0x03); // Oversampling ratio is 2^(x+1)
    adc.set_oversampling_shift(0b0000); // No bit shift after conversion, i.e. max value is 0xfff0
    adc.oversampling_enable(true);

    let mut pin = p.PA1;
    let mut vrefint = adc.enable_vrefint();
    let vrefint_sample = adc.blocking_read(&mut vrefint);
    info!("ADC reference value is {}", vrefint_sample);
    // let convert_to_millivolts = |sample| {
    //     // From stm32g0x0 datasheet: Reference voltage is 3V
    //     const VREFINT_MV: u32 = 3000; // mV

    //     (u32::from(sample) * VREFINT_MV / u32::from(vrefint_sample)) as u16
    // };
    // CHECK: Possibly check the VREFBUF config to make sure the internal reference voltage is used
    debug!("ADC setup complete!");

    let mut i2c_conf: i2c::Config = i2c::Config::default();
    i2c_conf.sda_pullup = true;
    i2c_conf.scl_pullup = true;
    i2c_conf.timeout = embassy_time::Duration::from_millis(1000);

    let i2c = I2c::new(
        p.I2C1,
        p.PA9,
        p.PA10,
        Irqs,
        p.DMA1_CH1,
        p.DMA1_CH2,
        Hertz(100_000),
        i2c_conf,
    );

    // let i2c_bus = NoopMutex::new(RefCell::new(i2c));
    // let i2c_bus = I2C_BUS.init(i2c_bus);
    // let i2c_dev1 = I2cDevice::new(i2c_bus);
    static I2C_BUS: StaticCell<I2cBus> = StaticCell::new();
    let i2c_bus = I2C_BUS.init(mutex::Mutex::new(i2c));
    let mut i2c_dev1 = I2cDevice::new(i2c_bus);

    let rheo = Rheo::new(i2c_bus);

    debug!("I2C setup complete!");

    let _ = bridge_enable.set_high();
    Timer::after_millis(100).await;

    let _ = ina_enable.set_low();
    Timer::after_millis(10).await;
    let _ = ina_enable.set_high();
    Timer::after_millis(100).await;
    let _ = bridge_enable.set_low();
    debug!("Enabled bridge and INA!");

    debug!("Entering loop!");
    loop {
        for i in 0..20 {
            // let i: u8 = 0;
            // Read rheo
            // match i2c_dev1
            //     .write_read(rheo_addr, &read_wiper0, &mut rx_buf)
            //     .await
            // {
            //     Ok(_) => debug!("Read {:?} from wiper0", rx_buf[1]),
            //     Err(err) => debug!("error: {:?}", err),
            // }
            // Timer::after_millis(100).await;

            // Write rheo
            _ = rheo.write(1).await;

            // loop {
            // increment wiper0 5 times
            // for i in 0..5 {
            // match i2c_dev1.write(rheo_addr, &increment_wiper0).await {
            //     Ok(_) => debug!("Incremented wiper0"),
            //     Err(err) => debug!("error: {:?}", err),
            // }
            // Timer::after_millis(10).await;
            // }

            // Timer::after_millis(100).await;

            // read wiper0
            // match i2c_dev1.write_read(rheo_addr, &read_wiper0, &mut rx_buf).await {
            //     Ok(_) => debug!("Read {:?} from wiper0", rx_buf[1]),
            //     Err(err) => debug!("error: {:?}", err),
            // }
            Timer::after_millis(100).await;

            // Read ADC value
            // Gain factor = 1 + 100k/R_rheo; R_rheo is R_wiper (75R) + N/128 * 10k, N in 0..128
            // Gain should then be in 11..1334
            let mut v = adc.blocking_read(&mut pin);
            // info!("{}", v);

            // led.toggle();
            // Timer::after_millis(100).await;

            // Autoscale reading
            while v < 8000 {
                _ = rheo.decrement().await;
                // Timer::after_millis(100).await;

                v = adc.blocking_read(&mut pin);
                match rheo.get_gain().await {
                    Ok(gain) => {
                        // let reading = convert_to_millivolts(v);
    
                        info!("{}, {}", v, gain);
                    }
                    Err(_) => {}
                }
            }

            while v > 50000 {
                _ = rheo.increment().await;
                // Timer::after_millis(100).await;

                v = adc.blocking_read(&mut pin);
                match rheo.get_gain().await {
                    Ok(gain) => {
                        // let reading = convert_to_millivolts(v);
    
                        info!("{}, {}", v, gain);
                    }
                    Err(_) => {}
                }
            }



            // Timer::after_millis(5).await;
        }
    }
}
