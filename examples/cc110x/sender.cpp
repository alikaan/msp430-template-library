#include "common.h"

static bool init_done = false;

static const uint8_t data[] = {
	0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x09, 0xff
};

int main(void)
{
	bool active = true;

	DCO::init();
	ACLK::init();
	SMCLK::init();
	MCLK::init();
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	SPI::init();
	CC1101::reset();
	CC1101::init(cc110x_default_init_values, ARRAY_COUNT(cc110x_default_init_values));
	init_done = true;
	while (1) {
		if (active) {
			LED_RED::set_high();
			CC1101::power_up();
			CC1101::tx_buffer(data, sizeof(data));
			CC1101::power_down();
			LED_RED::set_low();
			TIMEOUT::set_and_wait(5000);
			if (BUTTON::irq_raised()) {
				active = false;
			}
		} else {
			LED_RED::set_high();
			BUTTON::wait_for_irq();
			BUTTON::clear_irq();
			active = true;
		}
	}
}
