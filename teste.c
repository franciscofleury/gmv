#include <stdio.h>

int teste()
{
    static int t;
    t++;
    return t;
}

int main(void)
{
    for (int i = 0; i < 10; i++)
    {
        int a = teste();
        printf("%d\n", a);
    }
    return 0;
}