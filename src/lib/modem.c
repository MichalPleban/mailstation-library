
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

#define MS_MODEM_SCRATCH_TIMEOUT  100
#define MS_MODEM_INIT_TIMEOUT     500

#define ms_modem_wait() for(uint16_t i = 0; i < 4000; i++)

ms_modem_t ms_modem;

void ms_init_modem(void)
{
    uint8_t old_device = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_MODEM;

    ms_modem_regs[MS_MODEM_FCR] = 0x07; // Enable FIFO, reset RX/TX FIFOs
    ms_modem_regs[MS_MODEM_LCR] = 0x03; // 8 bits, no parity, one stop bit
    ms_modem_regs[MS_MODEM_MCR] = 0x0F; // Enable RTS/DSR, AUX outputs
    ms_modem_regs[MS_MODEM_IER] = 0x0F; // Enable RX and TX interrupts

    ms_port_8000_device = old_device;
}

void ms_reset_modem(void)
{
    // Pull reset line low.
    ms_port_modem_reset = 0x00;
    // Delay to reset the modem.
    ms_modem_wait();    

    // Pull reset line high.
    ms_port_modem_reset = 0x01;
    // Delay to allow the modem to power up.
    ms_modem_wait();    
}

bool ms_enable_modem(void)
{
    uint16_t timeout;
    bool ok;

    uint8_t old_device = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_MODEM;

    ms_modem.enabled = false;

    /*
     * Step 1: power on the modem - this is necessary on DET1 and DET2.
     */
    // Enable modem power.
    __critical {
        ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 & ~0x20;
    }
    // Delay until the power stabilizes.
    ms_modem_wait();    

    // Reset the modem.
    ms_reset_modem();

    /*
     * Step 2: check if modem actually exists, by reading the scratch register.
     * Modem is not present in the emulator.
     */
    timeout = 0;
    ok = false;
    while(timeout++ < MS_MODEM_SCRATCH_TIMEOUT)
    {
        ms_modem_regs[MS_MODEM_SCRATCH] = 0xA5;
        if(ms_modem_regs[MS_MODEM_SCRATCH] == 0xA5)
        {
            ok = true;
            break;
        }
    }
    if(!ok)
    {
        ms_port_8000_device = old_device;
        return false;
    }

    /*
     * Step 3: initialize the modem registers to known state.
     */
    ms_init_modem();

    /*
     * Step 4: wait for modem to become ready.
     * This is done by writing data to the modem and waiting for it to echo it back.
     */
    ok = false;
    while(timeout++ < MS_MODEM_INIT_TIMEOUT)
    {
        ms_modem_write('\r');
        if(ms_modem_regs[MS_MODEM_LSR] & 0x01)
        {
            ms_modem_read();
            ok = true;
            break;
        }
    }
    if(!ok)
    {
        ms_port_8000_device = old_device;
        return false;
    }

    ms_modem.enabled = true;

    /*
     * Step 5: determine modem type.
     */
    if(ms_modem.type == MS_MODEM_TYPE_NONE)
    {
        // Determine modem type.
        ms_modem.type = ms_interrogate_modem();

        // Reinitialize the modem to known state.
        ms_reset_modem();
        ms_init_modem();
        while(true)
        {
            ms_modem_write('\r');
            if(ms_modem_regs[MS_MODEM_LSR] & 0x01)
            {
                ms_modem_read();
                break;
            }
        }
    }

    ms_port_8000_device = old_device;
    return true;
}

void ms_disable_modem(void) __critical
{
    ms_port_shadow.gpio1 = ms_port_gpio1 = ms_port_shadow.gpio1 | 0x20;
}

uint8_t ms_modem_read(void)
{
    uint8_t byte;

    uint8_t old_device = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_MODEM;

    // Wait for data available
    while(!(ms_modem_regs[MS_MODEM_LSR] & 0x01));

    // Read byte
    byte = ms_modem_regs[MS_MODEM_BUFFER];

    ms_port_8000_device = old_device;

    return byte;
}

void ms_modem_write(uint8_t byte)
{
    uint8_t old_device = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_MODEM;

    // Wait for transmitter buffer empty
    while(!(ms_modem_regs[MS_MODEM_LSR] & 0x20));

    // Send byte
    ms_modem_regs[MS_MODEM_BUFFER] = byte;

    ms_port_8000_device = old_device;
}

uint8_t ms_interrogate_modem(void)
{
    uint8_t old_device = ms_port_8000_device;
    ms_port_8000_device = MS_DEVICE_MODEM;

    const char *output_buffer = "ATI3\r";
    char input_buffer[5];
    uint8_t input_index = 0;

    while(true)
    {
        // Send the next char from the command.
        if(*output_buffer)
        {
            ms_modem_write(*output_buffer);
            output_buffer++;
        }

        // Try to read a char from the modem.
        if(ms_modem_regs[MS_MODEM_LSR] & 0x01)
        {
            char c = ms_modem_read();
            if(c != '\r' && c != '\n')
            {
                input_buffer[input_index++] = c;
            }

            // The response starts to arrive
            if(input_index == 5)
            {
                ms_port_8000_device = old_device;
                switch(input_buffer[4])
                {
                    case 'V':
                        return MS_MODEM_TYPE_DET1;
                    case 'P':
                        return MS_MODEM_TYPE_DET2;
                    case 'A':
                        return MS_MODEM_TYPE_DET1D;
                    default:
                        return MS_MODEM_TYPE_UNKNOWN;
                }
            }
        }
    }
}

