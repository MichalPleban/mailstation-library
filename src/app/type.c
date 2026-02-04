#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "mailstation.h"

unsigned char last_second = 0xFF;

void draw_time(void)
{
    struct tm timeinfo;
    uint8_t cursor_x;
    uint8_t cursor_y;    
    char buffer[20];

    ms_get_time(&timeinfo);
    if(timeinfo.tm_sec == last_second) return;
    last_second = timeinfo.tm_sec;
    cursor_x = ms_screen.cursor_x;
    cursor_y = ms_screen.cursor_y;
    ms_position_cursor(32, 0);
    sprintf(buffer, "%02u:%02u:%02u\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    ms_draw_string(buffer, 8, true);
    ms_position_cursor(cursor_x, cursor_y);
}

void test_type(void)
{
    uint16_t key;
    uint8_t x;
    char c;
    char buffer[80];

    ms_put_string("Free type demo\n", false);
    for(x = 0; x < 40; x++) ms_screen_buffer[400+x] = 0xFF;
    ms_screen_update();
    ms_position_cursor(0, 2);
    while(true)
    {
        draw_time();
        key = ms_get_key(false);
        if(key == MS_KEY_NONE) continue;
        if(key == MS_KEY_POWER) ms_power_off();
        c = ms_translate_key(key);
        if(c) ms_put_char(c, true);
    }
}