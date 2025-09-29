
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

uint16_t lf_timer;

uint8_t read_port(uint8_t port) __naked
{
    __asm
    ld c, a
    in a, (c)
    ret
    __endasm;
    port;
}

void write_port(uint8_t port, uint8_t value) __naked
{
    __asm
    ld c, a
    out (c), l
    ret
    __endasm;
    port; value;
}

void increment_timer(void)
{
    lf_timer++;
}

void display_ports(void)
{
    char buffer[40];
    uint8_t base;
    uint32_t timer;
    static uint16_t time_diff;

    timer = ms_get_timer();
    ms_position_cursor(0, 0);
    ms_put_string("Ports: 00 01 02 03 04 05 06 07\n      -------------------------\n", false);
    for(base = 0; base < 0x30; base += 0x08)
    {
        sprintf(buffer, "   %02X: %02X %02X %02X %02X %02X %02X %02X %02X\n", base,
            read_port(base + 0), read_port(base + 1), read_port(base + 2), read_port(base + 3),
            read_port(base + 4), read_port(base + 5), read_port(base + 6), read_port(base + 7));
        ms_put_string(buffer, false);
    }
    sprintf(buffer, "\nTimers: HF=%08lu LF=%06u D=%03u", ms_get_timer(), lf_timer, time_diff);
    ms_put_string(buffer, false);
    sprintf(buffer, "\nKbd:    Flags=%02X Last=%02X", ms_keyboard.modifier_state, ms_keyboard.last_key);
    ms_put_string(buffer, false);
    sprintf(buffer, "\nOther:  Test=%02X Test2=%02X Screen=%02X", ms_screen.test, ms_port_gpio4, ms_screen_type());
    ms_put_string(buffer, false);
    ms_screen_update(NULL);
    time_diff = (ms_get_timer() - timer) & 0xFF;
    if(ms_keyboard.last_key == MS_KEY_POWER) ms_power_off();
}

void test_ports(void)
{
    lf_timer = 0;
    ms_add_irq_handler(MS_IRQ_LEVEL_1HZ, ms_current_device(), ms_current_bank(), increment_timer);
    ms_screen_clear();
    ms_enable_led(true);
    ms_cpu_speed(MS_CPU_12MHZ);
    while(true)
    {
        display_ports();
    }
}

