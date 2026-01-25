
#include "mailstation.h"

void ms_init(void)
{
    ms_modem.enabled = false;
    ms_modem.type = MS_MODEM_TYPE_NONE;
    ms_init_ports();
    ms_init_irq();
    ms_init_keyboard();
    ms_init_screen();
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
}

uint16_t ms_os_version(void)
{
    return *((uint16_t *)0x0036);
}
