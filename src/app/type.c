#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "mailstation.h"

void test_type(void)
{
    uint16_t key;
    char c;
    char buffer[80];

    ms_put_string("Free type demo:\n", true);
    while(true)
    {
        key = ms_get_key(true);
        if(key == MS_KEY_POWER) ms_power_off();
        c = ms_translate_key(key);
        if(c) ms_put_char(c, true);
    }
}