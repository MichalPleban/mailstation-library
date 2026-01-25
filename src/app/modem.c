
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

void display_modem(void)
{
//    ms_position_cursor(0, 0);
    for(uint8_t i = 0; i < 8; i++)
    {
        char buffer[4];
        sprintf(buffer, "%02X ", ms_modem_regs[i]);
        ms_put_string(buffer, false);
    }
    ms_put_string("\n", false);
    ms_screen_update();
}


void write_string(const char *str)
{
    while(*str)
    {
        ms_modem_write(*str);
        if(ms_modem_regs[MS_MODEM_LSR] & 0x01)
        {
            char c = ms_modem_read();
            ms_put_char(c, false);
            ms_screen_update();
        }
        str++;
    }
}

void test_modem(void)
{
    char buffer[64];

    ms_modem.type = MS_MODEM_TYPE_NONE;
    ms_cpu_speed(MS_CPU_12MHZ);
    ms_screen_clear();
    ms_put_string("Modem start\n", false);
    ms_screen_update();

    uint32_t start_time = ms_get_timer();
    ms_enable_modem();
    uint32_t end_time = ms_get_timer();

    sprintf(buffer, "Modem init time: %lu ticks\n", end_time - start_time);
    ms_put_string(buffer, false);
    ms_screen_update();

    sprintf(buffer, "Modem type: %u\n", ms_modem.type);
    ms_put_string(buffer, false);
    ms_screen_update();

//    display_modem();
//    ms_put_string("Modem initialized\n", false);
//    ms_screen_update();
    ms_port_8000_device = MS_DEVICE_MODEM;
    write_string("\rATI3\r\nATDT123456\r\n");

    while(true)
    {
        char c = ms_modem_read();
        ms_put_char(c, false);
        ms_screen_update();
    }
}
