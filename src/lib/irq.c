#include <stdint.h>

#include "mailstation.h"

#define NUM_IRQ_HANDLERS 8

static uint32_t ms_timer;

void ms_keyboard_scan(void);

static void ms_64hz_irq(void);

/*
 * Structure describing an IRQ handler.
 *
 * It allows defining the device (codeflash, dataflash, RAM) and bank where the
 *  handler resides.
 * The IRQ handler will browse through a list of these handlers and call the
 *  appropriate ones based on the IRQ level.
 */
typedef struct ms_irq_handler_t {
    uint8_t level;              // IRQ level + 1 (this way 0 denotes end of list)
    uint8_t port;               // Port number for device selection at 0x4000 or 0x8000
    uint8_t device;             // Device number (MS_DEVICE_*)
    uint8_t bank;               // Bank number
    void (*handler)(void);      // Pointer to the handler function
} ms_irq_handler_t;

ms_irq_handler_t ms_irq_handlers[NUM_IRQ_HANDLERS];
static uint8_t dummy_byte;

void ms_install_irq_handler(void);

void ms_init_irq(void)
{
    ms_irq_handlers[0].level = 0;
    dummy_byte = 0;
    ms_timer = 0;
    ms_install_irq_handler();
    ms_add_irq_handler(MS_IRQ_LEVEL_64HZ, ms_current_device(), ms_current_bank(), ms_64hz_irq);
}

static void ms_64hz_irq(void)
{
    ms_timer++;
    ms_keyboard_scan();
}

uint32_t ms_get_timer(void) __critical
{
    return ms_timer;
}

uint8_t ms_current_device(void) __naked
{
    __asm
    pop hl
    ld a, h
    and #0xC0
    cp #0x40
    jr z, .use_port_06
    cp #0x80
    jr nz, .return_device_zero
    in a, (0x08)
    jp (hl)
.return_device_zero:
    ld a, #0
    jp (hl)
.use_port_06:
    in a, (0x06)
    jp (hl)
    __endasm;
}

uint8_t ms_current_bank(void) __naked
{
    __asm
    pop hl
    ld a, h
    and #0xC0
    cp #0x40
    jr z, .use_port_05
    cp #0x80
    jr nz, .return_bank_zero
    in a, (0x07)
    jp (hl)
.return_bank_zero:
    ld a, #0
    jp (hl)
.use_port_05:
    in a, (0x05)
    jp (hl)
    __endasm;
}

bool ms_add_irq_handler(uint8_t level, uint8_t device, uint8_t bank, void (*handler)(void))
{
    unsigned char index;
    uint8_t addr_top = ((uint16_t)handler >> 8) & 0xC0;
    uint8_t port;

    /*
     * Determine which port to use for device selection:
     *  0x4000-0x7FFF: Port 0x05 (page) and 0x06 (device
     *  0x8000-0xCFFF: Port 0x07 (page) and 0x08 (device)
     *  others: no paging available, so just ose 0xFF and 0x00 (unused ports)
     */
    if(addr_top == 0x40)
        port = 0x06;
    else if(addr_top == 0x80)
        port = 0x08;
    else
        port = 0x00;

    for(index = 0; index < NUM_IRQ_HANDLERS; index++)
    {
        if(ms_irq_handlers[index].level == 0)
        {
            ms_irq_handlers[index].port = port;
            ms_irq_handlers[index].device = device;
            ms_irq_handlers[index].bank = bank;
            ms_irq_handlers[index].handler = handler;
            // Set the level last after filling up all the other fields.
            // This way the IRQ handler won't call an incomplete entry.
            ms_irq_handlers[index].level = level + 1;
            return true;
        }
    }
    return false;
}