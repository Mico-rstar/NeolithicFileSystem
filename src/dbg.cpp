#include "dbg.h"

#include <stdio.h>

void dbg::panic(char *s)
{
    printf("panic: ");
    printf(s);
    printf("\n");
    for (;;)
        ;
}