#include <stdio.h>
#include "lcd.h"

int main()
{
    printf("Hello, world!\r\n");
    printf("Read display MADCTL command is 0x%X\n", ST7789_RDDMADCTL);
    printf("Width: %u\n", LCD_WIDTH);
    printf("Height: %u\n", LCD_HEIGHT);
    return 0;
}
