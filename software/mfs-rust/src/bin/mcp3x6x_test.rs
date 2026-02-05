#![no_std]
#![no_main]

// embassy
use defmt::*;
use embassy_executor::Spawner;
use embassy_stm32::gpio::{Level, Input, Output, Speed};
use embassy_stm32::spi;
use embassy_stm32::spi::{Config, Spi};
use embassy_stm32::mode::Async;
use embassy_embedded_hal::shared_bus::blocking::spi::SpiDevice;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embassy_sync::blocking_mutex::Mutex;
use embassy_stm32::time::Hertz;
use embassy_sync::mutex;
use embassy_time::Timer;


use core::cell::RefCell;
use static_cell::StaticCell;

// use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

// MCP3562R library
use mcp3x6x::{ClkSel, Config0, Config1, FastCommand, Irq, MCP3x6x, ToVoltageConverter24bit};

// use 3.3V as Vref+ and 0V as Vref-
const TO_VOLT: ToVoltageConverter24bit = ToVoltageConverter24bit::new(3.3, 0.0, mcp3x6x::Gain::X1);

// Mutexed SPI Bus type
type SpiMutex = mutex::Mutex<NoopRawMutex, Spi<'static, Async>>;


#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    let p = embassy_stm32::init(Default::default());
    info!("Start test connecting to MCP3562R!");

    // SPI setup in two stages: Inner SpiBus object and outer SpiDevice object
    // Setup SpiBus
    let mut spi_config = Config::default();
    spi_config.frequency = Hertz(4_000_000);

    let mut spi_peripheral: Spi<'_, Async> = Spi::new( p.SPI2, // SPI peripheral
                                            p.PB13, // SCK pin
                                            p.PB15, // MOSI pin
                                            p.PB14, // MISO pin
                                            p.DMA1_CH4, // DMA channel transmit
                                            p.DMA1_CH3, // DMA channel receive
                                            spi_config);
    //TODO: Setup interrupt for pin?

    // Setup SpiDevice
    // Chip select pin
    let cs = Output::new(p.PB12, Level::High, Speed::VeryHigh); 
    // Wrap into SpiDevice
    // static SPI_CELL: StaticCell<SpiMutex> = StaticCell::new();
    // let spi_bus_mx = SPI_CELL.init(mutex::Mutex::new(spi_bus));
    let spi_bus = Mutex::<NoopRawMutex, _>::new(RefCell::new(spi_peripheral));
    // let spi_dev = SpiDevice::<_, NoopRawMutex>::new(spi_bus, cs);
    let spi_dev = SpiDevice::new(&spi_bus, cs);


    // spi is a struct implementing embedded_hal::spi::SpiDevice.
    // irq is an input pin attached to the IRQ pin of the ADC.
    let irq_pin = Input::new(p.PC13, embassy_stm32::gpio::Pull::Up);

    let mut adc = MCP3x6x::new(spi_dev);

    // use internal clock
    let config0 = Config0::default().with_clk_sel(ClkSel::InternalClock);
    match adc.write_register(config0){
        Ok(_) => debug!("Configured ADC Config0"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // Set oversampling to 128
    let config1 = mcp3x6x::Config1::default().with_osr(mcp3x6x::Osr::Osr128);
    match adc.write_register(config1){
        Ok(_) => debug!("Configured ADC Config1"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // Set oversampling to 128
    let config2 = mcp3x6x::Config2::default().with_gain(mcp3x6x::Gain::X32);
    match adc.write_register(config2){
        Ok(_) => debug!("Configured ADC Config2"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // Set to single conversion mode
    let config3 = mcp3x6x::Config3::default().with_conv_mode(mcp3x6x::ConvMode::OneShotStandby);
    match adc.write_register(config3){
        Ok(_) => debug!("Configured ADC Config3"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // disable en_stp
    let irq = Irq::default().with_en_stp(false);
    match adc.write_register(irq){
        Ok(_) => debug!("Configured ADC Interrupt"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // Set to single conversion mode
    let mux = mcp3x6x::Mux::default()
                    .with_vin_p(mcp3x6x::MuxInput::Ch0)
                    .with_vin_n(mcp3x6x::MuxInput::Ch1);
    // let mux = mcp3x6x::Mux::default()
    //                 .with_vin_p(mcp3x6x::MuxInput::Ch2)
    //                 .with_vin_n(mcp3x6x::MuxInput::Refn);
    match adc.write_register(mux){
        Ok(_) => debug!("Configured ADC Mux"),
        Err(err) => debug!("error: {:?}", err),    
    }

    // be ready for conversions
    let _ = adc.fast_command(FastCommand::Standby).unwrap();

    loop {
        let _ = adc.fast_command(FastCommand::ConversionStart);
        while irq_pin.is_high() {}
        let sample = adc.read_24_bit_adc_data().unwrap();
        let voltage = TO_VOLT.to_volt(sample);
        info!("ADC reads {} equivalent to {}V", sample, voltage);
        Timer::after_millis(1000).await;
    }
}

