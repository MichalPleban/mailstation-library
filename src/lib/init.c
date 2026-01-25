
#include "mailstation.h"

#include <stdint.h>

static void ms_os_check(void);

void ms_init(void)
{
    ms_modem.enabled = false;
    ms_modem.type = MS_MODEM_TYPE_NONE;
    ms_init_ports();
    ms_init_irq();
    ms_init_keyboard();
    ms_init_screen();
    /*
    ms_enable_modem();
    if(ms_modem.type == MS_MODEM_TYPE_DET2 || ms_modem.type == MS_MODEM_TYPE_DET1D || ms_screen.type == MS_SCREEN_NEW)
    {
        ms_keyboard.type = MS_KEYBOARD_NEW;
    }
    else
    {
        ms_keyboard.type = MS_KEYBOARD_OLD;
    }
    ms_disable_modem();
    */
   ms_os_check();
}

uint16_t ms_os_version(void)
{
    return *((uint16_t *)0x0036);
}

static void ms_os_check(void)
{
    uint8_t os_major = *((uint8_t *)0x0037);
    if(os_major <= 3)
    {
        ms_modem.type == MS_MODEM_TYPE_DET1;
        ms_keyboard.type = MS_KEYBOARD_OLD;
    }
    else
    {
        ms_keyboard.type = MS_KEYBOARD_NEW;
        if(ms_screen.type == MS_SCREEN_NEW)
        {
            ms_modem.type = MS_MODEM_TYPE_DET1D;
        }
        else
        {
            ms_modem.type = MS_MODEM_TYPE_DET2;
        }
    }
}
