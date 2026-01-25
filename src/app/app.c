#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

//void test_ports(void);
//void test_modem(void);
//void test_screen(void);
void test_type(void);

void main(void)
{
    ms_init();
	test_type();
}
