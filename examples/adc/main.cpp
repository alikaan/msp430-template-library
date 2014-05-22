#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>
#include <adc.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_PORT_T<1, RX, TX> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef ADC10_T<ADC10OSC, 0, 0, 1 << BIT3> ADC_CHANNEL_3;

typedef TIMEOUT_T<WDT> TIMEOUT;

int adc_read_ref(int channel, int ref)
{
	int r;

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ref;
	ADC10CTL1 = ADC10SSEL_3 + channel;
	__delay_cycles (128);
	ADC10CTL0 |= ENC + ADC10SC;
        while( ADC10CTL1 & ADC10BUSY ) ;

	r = ADC10MEM;
	ADC10CTL0 &= ~ENC;

	return r;
}

int adc_read_single(int channel)
{
	int r;

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;
	ADC10CTL1 = ADC10SSEL_3 + channel;
	ADC10CTL0 |= ENC + ADC10SC;
        while( ADC10CTL1 & ADC10BUSY ) ;

	r = ADC10MEM;
	ADC10CTL0 &= ~ENC;

	return r;
}

unsigned read_voltage(void)
{
	unsigned adc, voltage;

	ADC10CTL1 = INCH_11 | ADC10DIV_3 | ADC10SSEL_3;
	ADC10CTL0 = ADC10SHT_3 | ADC10ON | ENC | REF2_5V | ADC10SC | REFON | SREF_1;
	while (ADC10CTL1 & ADC10BUSY) ;
	adc = ADC10MEM;
	ADC10CTL0 &= ~ENC;
	voltage = ((unsigned long) adc * 5L);

	return voltage;
}

int main(void)
{
	unsigned int vcc_milli, adc, adc_ref, adc_norm, r;

	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	UART::init();
	WDT::enable_irq();
	printf<UART>("ADC example:\n");
	while (1) {
		ADC_CHANNEL_3::init();
		vcc_milli = read_voltage(); // (unsigned long) (adc_read_ref(INCH_11, REF2_5V) * 5L * 125L / 128L);
		adc = adc_read_single(INCH_3);
		adc_ref = adc_read_ref(INCH_3, REF2_5V);
		adc_norm = ((unsigned long) adc * (unsigned long) vcc_milli) >> 10;
		r = 10000L * (unsigned long) adc_norm / (unsigned long) (vcc_milli - adc_norm);
		printf<UART>("\rADC: no ref: %d ref 2.5: %d norm: %d r: %d V: %d\033[K",
			     adc, adc_ref, adc_norm, r, vcc_milli);
		ADC_CHANNEL_3::disable();
		TIMEOUT::set_timeout(200);
		enter_idle<TIMEOUT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::timeout_triggered()) exit_idle();
}