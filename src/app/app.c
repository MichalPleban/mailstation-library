#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "mailstation.h"

void test_ports(void);

void main(void)
{
    ms_init();
	ms_put_string("Mailstation test application\nThis is a second line\nAnd a third\n", true);
	test_ports();
}
