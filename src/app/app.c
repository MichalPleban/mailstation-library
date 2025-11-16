#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

void test_ports(void);

void main(void)
{
    ms_init();
	test_ports();
}
