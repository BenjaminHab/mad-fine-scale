use embassy_sync::mutex;
use embassy_sync::blocking_mutex::raw::NoopRawMutex;
use embedded_hal_async::i2c::I2c as _;
use embassy_embedded_hal::shared_bus::asynch::i2c::I2cDevice;
use embassy_stm32::i2c::{self, I2c};
use embassy_stm32::mode::Async;
use defmt::*;
use core::result::Result::{self, Ok, Err};





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

type I2cBus = mutex::Mutex<NoopRawMutex, I2c<'static, Async, i2c::Master>>;

pub struct Rheo {
    bus: &'static I2cBus,
}

impl Rheo {
    pub fn new(bus: &'static I2cBus) -> Rheo {
        Rheo { bus: bus }
    }

    pub async fn read(&self) -> Result<u8, ()> {
        let mut i2c_dev = I2cDevice::new(self.bus);
        let mut rx_buf: [u8; 2] = [0x00, 0x00];
        match i2c_dev
            .write_read(rheo_addr, &read_wiper0, &mut rx_buf)
            .await
        {
            Ok(_) => {
                debug!("Read {:?} from wiper0", rx_buf[1]);
                Ok(rx_buf[1])
            }
            Err(err) => {
                error!("error: {:?}", err);
                Err(())
            }
        }
    }

    pub async fn increment(&self) -> Result<(), ()> {
        let mut i2c_dev = I2cDevice::new(self.bus);
        match i2c_dev.write(rheo_addr, &increment_wiper0).await {
            Ok(_) => {
                debug!("Incremented wiper0");
                Ok(())
            }
            Err(err) => {
                error!("error: {:?}", err);
                Err(())
            }
        }
    }

    pub async fn decrement(&self) -> Result<(), ()> {
        let mut i2c_dev = I2cDevice::new(self.bus);
        match i2c_dev.write(rheo_addr, &decrement_wiper0).await {
            Ok(_) => {
                debug!("Decremented wiper0");
                Ok(())
            }
            Err(err) => {
                error!("error: {:?}", err);
                Err(())
            }
        }
    }

    pub async fn write(&self, data: u8) -> Result<(), ()> {
        let mut i2c_dev = I2cDevice::new(self.bus);
        let send_buffer: [u8; 2] = [rheo_volatile_wiper0_addr << 4 | rheo_write << 2, data];
        match i2c_dev.write(rheo_addr, &send_buffer).await {
            Ok(_) => {
                debug!("Wrote {:?} to wiper0", &send_buffer[1]);
                Ok(())
            }
            Err(err) => {
                error!("error: {:?}", err);
                Err(())
            }
        }
    }

    fn calculate_gain(wiper_position: u8) -> f32 {
        1.0f32 + 100.0e3f32 / (75.0f32 + 1e4f32 * ((wiper_position as f32) / 128.0f32))
    }

    pub async fn get_gain(&self) -> Result<f32, ()> {
        match self.read().await {
            Ok(wiper_position) => Ok(Rheo::calculate_gain(wiper_position)),
            Err(err) => {
                error!("Could not read gain factor from rheo!");
                Err(())
            }
        }
    }
}
