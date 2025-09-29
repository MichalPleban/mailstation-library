
#include <stdbool.h>

#include "mailstation.h"

ms_port_shadow_t ms_port_shadow;

void ms_init_ports(void)
{
    // Screen at 0xC000
    ms_port_screen_page = 0x00;

    // All interrupts enabled.
    ms_port_shadow.irq_mask = ms_port_irq_mask = 0xFF;

    // All keyboard matrix rows set to 0
    ms_port_keyboard = 0x00;

    // LCD on, LED off, LCD column strobe off, keyboard rows 8 & 9 set to 0
    ms_port_shadow.gpio1 = ms_port_gpio1 = 0xA4;
    ms_port_gpio1_dir = 0xFF;

    // All printer port conrol signals set to 1
    ms_port_shadow.gpio2 = ms_port_gpio2 = 0x0F;
    ms_port_gpio2_dir = 0x0F;

    // Disable power off lines (obviously)
    ms_port_shadow.gpio4 = ms_port_gpio4 = 0x00;
    ms_port_gpio4_dir = 0x3;

    // Printer port data lines
    ms_port_shadow.gpio5 = ms_port_gpio5 = 0x00;
    ms_port_gpio5_dir = 0xFF;

    // Medium CPU speed (10 MHz)
    ms_port_shadow.cpu_speed = ms_port_cpu_speed = 0x32;

    // RTC test, must be set to 0
    ms_port_rtc_test = 0x00;
    // Enable timer, disable alarm, page 0 (current time)
    ms_port_rtc_control = 0x08;
    // Disable 1Hz and 16Hz interrupts, don't reset anything
    ms_port_rtc_reset = 0x0C;

    // Interrupt every 1 second
    ms_port_irq_speed = 0x80;
}

void ms_power_off(void)
{
    ms_port_gpio4 = ms_port_shadow.gpio4 | MS_GPIO4_POWER_OFF;
}

void ms_enable_lcd(bool on)
{
    if(on)
        ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 | MS_GPIO1_LCD_ON;
    else
        ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 & ~MS_GPIO1_LCD_ON;
}

void ms_enable_led(bool on)
{
    if(on)
        ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 | MS_GPIO1_LED_ON;
    else
        ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 & ~MS_GPIO1_LED_ON;
}

void ms_cpu_speed(uint8_t speed)
{
    ms_port_shadow.cpu_speed = ms_port_cpu_speed = (ms_port_shadow.cpu_speed & 0x0F) | speed;
}

void ms_irq_speed(uint8_t speed)
{
    ms_port_irq_speed = 0x80 | speed;
}
