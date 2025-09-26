
#include <stdint.h>
#include <string.h>

#include "mailstation.h"

ms_screen_t ms_screen;

static const uint8_t ms_reverse_bytes[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 
    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 
    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 
    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 
    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 
    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1, 
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 
    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 
    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD, 
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 
    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

void ms_init_screen(void)
{
    ms_screen.type = ms_screen_type();
    ms_screen_clear();
}

void ms_screen_clear(void)
{
    memset(ms_screen_buffer, 0x00, sizeof(ms_screen_buffer));
    ms_screen.cursor_x = 0;
    ms_screen.cursor_y = 0;
    ms_screen_update(NULL);
}

void ms_position_cursor(uint8_t x, uint8_t y)
{
    ms_screen.cursor_x = x;
    ms_screen.cursor_y = y;
}

void ms_draw_char(char c, bool refresh)
{
    uint8_t *dest = ms_screen_buffer + ms_screen.cursor_x + ms_screen.cursor_y*320;
    unsigned char i;

    for(i = 0; i < 8; i++)
    {
        *dest = ms_screen_font[(uint8_t)c*8 + i];
        dest += 40;
    }
    if(ms_screen.type == MS_SCREEN_OLD && refresh)
    {
        ms_screen_update_column(ms_screen.cursor_x, ms_screen_buffer + ms_screen.cursor_x*128);
    }
}

void ms_draw_string(const char *str, int length, bool refresh)
{
    int i;
    
    if(!length) length = strlen(str);
    for(i = 0; i < length; i++)
    {
        ms_draw_char(str[i], refresh);
        ms_advance_cursor(false, false);
    }
}

void ms_screen_scroll(bool refresh)
{
    volatile uint8_t *src_ptr = ms_screen_buffer;

    for(unsigned char i = 0; i < 40; i++)
    {
        memcpy(src_ptr + 8, src_ptr, 128 - 8);
        memset(src_ptr + 128 - 8, 0x00, 8);
        src_ptr += 128;
    }
    if(refresh) ms_screen_update(NULL);
}

void ms_advance_cursor(bool scroll, bool refresh_on_scroll)
{
    ms_screen.cursor_x++;
    if(ms_screen.cursor_x >= 40)
    {
        ms_screen.cursor_x = 0;
        ms_screen.cursor_y += 1;
        if(ms_screen.cursor_y >= 16)
        {
            ms_screen.cursor_y = 16;
            if(scroll) ms_screen_scroll(refresh_on_scroll);
        }
    }
}

void ms_put_char(char c, bool refresh)
{
    switch(c)
    {
        case '\r':
            ms_screen.cursor_x = 0;
            break;
        case '\n':
            ms_screen.cursor_x = 0;
            ms_screen.cursor_y++;
            if(ms_screen.cursor_y >= 16)
            {
                ms_screen.cursor_y = 16;
                ms_screen_scroll(refresh);
            }
            break;
        default:
            ms_draw_char(c, refresh);
            ms_advance_cursor(true, refresh);
    }
}

void ms_put_string(const char *str, bool refresh)
{
    while(*str)
    {
        ms_put_char(*str, refresh);
        str++;
    }
}

void ms_screen_update(uint8_t *buffer)
{
    unsigned char screen_column;
    uint8_t *ptr;

    if(ms_screen.type == MS_SCREEN_NEW) return;

    if(buffer == NULL) buffer = ms_screen_buffer;

    ptr = buffer;
    for(screen_column = 0; screen_column < 40; screen_column++)
    {
        ms_screen_update_column(screen_column, ptr);
        ptr += 1;
    }
    // TODO: why is this needed? Without it the first column is not updated properly.
    ms_screen_update_column(0, buffer);
}

void ms_screen_update_column(uint8_t column, uint8_t *buffer)
{
    uint8_t device_8000;
    uint8_t lcd_device = MS_DEVICE_LCD_LEFT;
    volatile uint8_t *lcd_ptr = (uint8_t *)0x8038;
    unsigned char i;

    if(ms_screen.type == MS_SCREEN_NEW) return;

    if(column >= 20)
    {
        lcd_device = MS_DEVICE_LCD_RIGHT;
        column -= 20;
    }

    device_8000 = ms_port_8000_device;
    ms_port_8000_device = lcd_device;

    __asm__("di");
    ms_port_gpio1 = ms_port_shadow.gpio1 & ~MS_GPIO1_LCD_COLUMN;
    *lcd_ptr = 19-column;
    ms_port_gpio1 = ms_port_shadow.gpio1 | MS_GPIO1_LCD_COLUMN;
    __asm__("ei");

    for(i = 0; i < 128; i++)
    {
        *lcd_ptr = ms_reverse_bytes[*buffer];
        buffer += 40;
        lcd_ptr++;
    }

    ms_port_8000_device = device_8000;
}

/*
 * Detect screen and hardware type.
 * The "new" Mailstations have the LCD bitmap mapped directly to the CPU address space,
 *  while the "old" ones require writing to the memory-mapped LCD device.
 * We try writing data to this device and reding it back. If the data matches,
 *  the device is present and thus it's an "old" Mailstation.
 */
uint8_t ms_screen_type(void)
{
    uint8_t device_8000;
    uint8_t read_back;
    volatile uint8_t *lcd_ptr = (uint8_t *)0x8000;

    device_8000 = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_LCD_LEFT;

    __asm__("di");
    ms_port_gpio1 = ms_port_shadow.gpio1 & ~MS_GPIO1_LCD_COLUMN;
    *lcd_ptr = 0;
    ms_port_gpio1 = ms_port_shadow.gpio1 | MS_GPIO1_LCD_COLUMN;
   __asm__("ei");    

    lcd_ptr[0] = 0x5A;
    lcd_ptr[1] = 0xA5;
    read_back = lcd_ptr[0];

    ms_port_8000_device = device_8000;

    return read_back == 0x5A ? MS_SCREEN_OLD : MS_SCREEN_NEW;
}

