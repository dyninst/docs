#include <stdio.h>

void InterestingProcedure()
{
    printf("\t This is an interesting procedure\n");
}

int main()
{
    printf("Hello, world!\n");

    int i; 
    for (i = 0; i < 10; i++)
        InterestingProcedure();

    return 0;
}

