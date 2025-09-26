
#include "mailstation.h"

void ms_init(void)
{
    ms_init_ports();
    ms_init_irq();
    ms_init_keyboard();
    ms_init_screen();
}