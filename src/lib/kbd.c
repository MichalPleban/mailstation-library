#include <stdint.h>

#include "mailstation.h"

#define INITIAL_REPEAT_DELAY 24
#define SUBSEQUENT_REPEAT_DELAY 4

ms_keyboard_t ms_keyboard;

static char ms_keycode_translation[] = {
    // Without Shift
    0x00, 0x1B, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, // 00-07
    0x00, 0x00, 0x00, '@',  0x00, 0x00, 0x00, 0x00, // 10-17
    '`',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  // 20-27
    '8',  '9',  '0',  '-',  '=',  '\b', '\\', 0x00, // 30-37
    '\t', 'q',  'w',  'e',  'r',  't',  'y',  'u',  // 40-47
    'i',  'o',  'p',  '[',  ']',  ';',  '\'', '\n', // 50-57
    0x00, 'a',  's',  'd',  'f',  'g',  'h',  'j',  // 60-67
    'k',  'l',  ',',  '.',  '/',  0x00, 0x00, 0x00, // 70-77
    0x00, 'z',  'x',  'c',  'v',  'b',  'n',  'm',  // 80-87
    0x00, 0x00, 0x00, 0x00, ' ',  0x00, 0x00, 0x00, // 90-97
    // With Shift
    0x00, 0x1B, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, // 00-07
    0x00, 0x00, 0x00, '@',  0x00, 0x00, 0x00, 0x00, // 10-17
    '~',  '!',  '@',  '#',  '$',  '%',  '^',  '&',  // 20-27
    '*',  '(',  ')',  '_',  '+',  '\b', '\\', 0x00, // 30-37
    '\t', 'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  // 40-47
    'I',  'O',  'P',  '{',  '}',  ':',  '"',  '\n', // 50-57
    0x00, 'A',  'S',  'D',  'F',  'G',  'H',  'J',  // 60-67
    'K',  'L',  '<',  '>',  '?',  0x00, 0x00, 0x00, // 70-77
    0x00, 'Z',  'X',  'C',  'V',  'B',  'N',  'M',  // 80-87
    0x00, 0x00, 0x00, 0x00, ' ',  0x00, 0x00, 0x00, // 90-97
};

void ms_init_keyboard(void)
{
    ms_keyboard.buffer_head = 0;
    ms_keyboard.buffer_tail = 0;
    ms_keyboard.modifier_state = 0;
//    ms_keyboard.last_key = 0xFF;
    ms_keyboard.key_pressed = false;
    ms_keyboard.caps_pressed = false;
//    ms_keyboard.repeat_delay = 0;
}

void ms_keyboard_scan(void)
{
    uint8_t row, column;
    uint16_t row_mask;
    uint8_t column_mask;
    uint8_t key;
    uint8_t key_found = 0xFF;

    // Remember Caps Lock state to detect release
    bool caps_was_pressed = ms_keyboard.caps_pressed;
    ms_keyboard.caps_pressed = false;

    // Clear modifier state, it will be updated in the loop below
    ms_keyboard.modifier_state &= ~(MS_MODIFIER_SHIFT | MS_MODIFIER_FN);

    // Since all rows are grounded, we can just check if any key is pressed
    if(ms_port_keyboard != 0xFF)
    {
        // Set all rows to 1 to stabilize the first reading.
        ms_port_keyboard = 0xFF;
        ms_port_gpio1 = ms_port_shadow.gpio1 | 0x03;
        
        // 10 rows, row 1 set to 0 initially
        row_mask = 0x3FE;

        for(row = 0; row < 10; row++)
        {
            // First 8 rows are defined in the keyboard register
            ms_port_keyboard = row_mask & 0xFF;
            // Last two rows are defined in the GPIO1 port, bits 0 and 1
            ms_port_gpio1 = ms_port_shadow.gpio1 & 0xFC | ((row_mask >> 8) & 0x03);

            // Read the column mask, 0 bits denote pressed keys
            column_mask = ms_port_keyboard;

            // Set all rows back to 1.
            // This is needed because we are reading the column mask right after
            //  changing the row mask, and the hardware seems to need a short time
            //  to stabilize the column readings.
            ms_port_keyboard = 0xFF;
            ms_port_gpio1 = ms_port_shadow.gpio1 | 0x03;

            // Only if something is pressed in this row
            if(column_mask != 0xFF)
            {
                // Check each column in this row
                for(column = 0; column < 8; column++)
                {
                    // If this column is active (0), we found a key
                    if(!(column_mask & 0x01))
                    {
                        // Calculate the scancode from row and column
                        key = (row << 4) + column;

                        // Handle modifier keys
                        if(key == MS_KEY_LSHIFT || key == MS_KEY_RSHIFT)
                            ms_keyboard.modifier_state |= MS_MODIFIER_SHIFT;
                        else if(key == MS_KEY_FN)
                            ms_keyboard.modifier_state |= MS_MODIFIER_FN;
                        else if(key == MS_KEY_CAPS)
                            ms_keyboard.caps_pressed = true;
                        else
                            key_found = key;
                    }

                    // Move to the next column
                    column_mask >>= 1;
                }
            }

            // Left shift the row mask to enable the next row
            row_mask = (row_mask << 1) | 0x01;
        }
    }

    // Check for power button
    if(!(ms_port_gpio2 & MS_GPIO2_POWER_BUTTON))
        key_found = MS_KEY_POWER;

    if(caps_was_pressed && !ms_keyboard.caps_pressed)
    {
        // Caps Lock was just released, toggle the caps state
        ms_keyboard.modifier_state ^= MS_MODIFIER_CAPS;
    }

    if(key_found != 0xFF)
    {
        // A non-modifier key is pressed
        if(!ms_keyboard.key_pressed || key_found != ms_keyboard.last_key)
        {
            // New key, set repeat delay to longer initial delay
            ms_keyboard.last_key = key_found;
            ms_keyboard.key_pressed = true;
            ms_keyboard.repeat_delay = INITIAL_REPEAT_DELAY; // Initial delay before repeating
        }
        else if(ms_keyboard.key_pressed)
        {
            // Same key is still pressed
            if(ms_keyboard.repeat_delay > 0)
            {
                // Still in the delay period
                ms_keyboard.repeat_delay--;
                key_found = 0xFF; // Don't register the key yet
            }
            else
            {
                // Repeat the key
                ms_keyboard.key_pressed = true;
                ms_keyboard.repeat_delay = SUBSEQUENT_REPEAT_DELAY; // Faster repeat delay
            }
        }

        if(key_found != 0xFF)
        {
            // Add the key to the buffer if there's space
            uint8_t next_head = (ms_keyboard.buffer_head + 1) % MS_KEYBOARD_BUFFER_SIZE;
            if(next_head != ms_keyboard.buffer_tail)
            {
                ms_keyboard.buffer[ms_keyboard.buffer_head] = key_found | (ms_keyboard.modifier_state << 8);
                ms_keyboard.buffer_head = next_head;
            }
        }
    }
    else
    {
        // No key pressed
        ms_keyboard.key_pressed = false;
    }

    // Set all rows to 0 again, also if something else changed them
    ms_port_keyboard = 0x00;
    ms_port_gpio1 = ms_port_shadow.gpio1 & 0xFC;
}

bool ms_keyboard_key_available(void) __critical
{
    return ms_keyboard.buffer_head != ms_keyboard.buffer_tail;
}

uint16_t ms_keyboard_get_key(bool wait) __critical
{
    while(ms_keyboard.buffer_head == ms_keyboard.buffer_tail)
    {
        if(!wait) return MS_KEY_NONE;
        __asm__("halt");
    }

    uint16_t key = ms_keyboard.buffer[ms_keyboard.buffer_tail];
    ms_keyboard.buffer_tail = (ms_keyboard.buffer_tail + 1) % MS_KEYBOARD_BUFFER_SIZE;
    return key;
}

char ms_keyboard_translate_key(uint16_t keycode)
{
    bool shift = (keycode >> 8) & MS_MODIFIER_SHIFT;
    uint8_t key = (keycode & 0x000F) | ((keycode & 0x00F0) >> 1);
    char c = ms_keycode_translation[key + (shift ? 80 : 0)];

    // Handle Caps Lock for alphabetic characters
    if((keycode >> 8) & MS_MODIFIER_CAPS)
    {
        if(c >= 'a' && c <= 'z')
            c -= 32; // Convert to uppercase
        else if(c >= 'A' && c <= 'Z')
            c += 32; // Convert to lowercase
    }

    return c;
}