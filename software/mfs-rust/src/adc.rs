use defmt::*;
use core::result::Result::{self, Ok, Err};
use embassy_stm32::{adc::Adc, init};

type AdcDev = mutex::Mutex<NoopRawMutex, Adc<'static, AnyAdcChannel>>;

pub struct ADC {
    adcdev: AdcDev,
    adcper: Adc<'static, AnyAdcChannel>,
    defaultchannel: AdcChannel
}

impl ADC {
    pub fn new(adcPeripheral: Adc<'static, AnyAdcChannel>, defaultchannel: AdcChannel) -> ADC{
        static ADC_DEV: StaticCell<AdcDev> = StaticCell::new();
        adcdev = ADC_DEV.init(mutex::Mutex::new(adc));
        adc = ADC{adcper: adcPeripheral, adcdev: adcdev, defaultchannel: defaultchannel};
        adc.init();
        adc
    }

    fn init(&self) -> _ {
        self.adcper.set_sample_time(SampleTime::CYCLES79_5);
        self.adcper.set_oversampling_ratio(0x03); // Oversampling ratio is 2^(x+1)
        self.adcper.set_oversampling_shift(0b0000); // No bit shift after conversion, i.e. max value is 0xfff0
        self.adcper.oversampling_enable(true);
    }
    pub async fn read(&self) -> u16{

    }
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

fn read_raw_info() -> _ {
        // Raw read from memory
        const ADC: *mut u16 = (0x40012400) as *mut u16;
        const ADC_DR: *mut u16 = ADC.wrapping_add(0x40);
        const VREFINT: *mut u16 = (0x1FFF75AA) as *mut u16;
        unsafe {
            info!("VREFINT_LOW {}", core::ptr::read_volatile(VREFINT));
            info!("ADC_DR {}", core::ptr::read_volatile(ADC_DR));
        }
}