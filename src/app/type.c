#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "mailstation.h"

void test_type(void)
{
    uint16_t key;
    char c;
    char buffer[80];

    ms_screen_clear();
    sprintf(buffer, "Mailstation OS Version: %02X\n", ms_os_version());
    ms_put_string(buffer, true);
    while(true)
    {
        key = ms_get_key(true);
        c = ms_translate_key(key);
        if(c) ms_put_char(c, true);
    }
}