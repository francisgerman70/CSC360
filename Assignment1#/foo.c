/* flags.c -- illustrates some formatting flags */
#include <stdio.h>
int main(void)
{
    int x = 0;
    while(x<10){
        printf("%x %X %#x\n", 31, 31, 31);
        sleep(6);
        printf("**%d**% d**% d**\n", 42, 42, -42);
        sleep(10);
        printf("**%5d**%5.3d**%05d**%05.3d**\n", 6, 6, 6, 6);
        
        x++;
        
    }
return 0;
}