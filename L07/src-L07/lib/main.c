#include <stdio.h>

#include "myutil.h"


int main()
{
    printf("Inside main()\n");

    /* use a function from each object file that is in the library */
    util_file();
    util_net();
    util_math();

    return 0;
}

