#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>

typedef ACLK_T<ACLK_SOURCE_LFXT1CLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;

typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;

typedef GPIO_OUTPUT_T<1, 4, HIGH> CS;
typedef GPIO_MODULE_T<1, 5, 3> SCK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;

typedef GPIO_PORT_T<1, LED_RED, RX, TX, CS, SCK, MISO, MOSI> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
typedef SPI_T<USCI_B, 0, SMCLK> SPI;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef ALARM_T<WDT> ALARM;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT2::init();
	LED_RED::set_low();
	CS::set_high();
	SPI::init();
	UART::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();

		CS::set_low();
		SPI::transfer((uint8_t *) "abc", 3);
		CS::set_high();

		ALARM::set_alarm(1000);
		printf<UART>("WDT frequency: %d Alarm: %d\n", WDT::frequency, ALARM::alarm);
		enter_idle<WDT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (ALARM::alarm_triggered()) exit_idle();
}

void usci_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}

