
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mailstation.h"

void test_screen(void)
{
    uint8_t byte = 0x80;

    __asm__("di");
    while(true)
    {
        memset((void*)ms_screen_buffer, byte, sizeof(ms_screen_buffer));
        ms_screen_update();
//        for(uint16_t i = 0; i < 10000; i++);
        byte >>= 1;
        if(byte == 0) byte = 0x80;
    }
}