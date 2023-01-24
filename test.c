#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void)
{   
    char answer[20] = {0};
    char myanswer[20] = {0};
    myanswer[0] = 'b';
    if (memcmp(myanswer, answer, 1) == 0)
    {
        printf("Equal");
    }
    else
    {
        printf("Not equal");
    }
    
    return 0;
}