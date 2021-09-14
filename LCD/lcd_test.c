#include <stdio.h>
#include "lcd.h"

int main()
{
    printf("This simple program is just so I can test the LCD panel.\n");
    printf("Read display MADCTL command is 0x%X\n", ST7789_RDDMADCTL);
    printf("Width: %u\n", LCD_WIDTH);
    printf("Height: %u\n", LCD_HEIGHT);

    // wiringPiSetupGpio() is required for wiringPi to actually work, it should
    // probably be one of the first things we do in our main function of our 
    // final main program. To use, the program must run with root privilege.
    // Should return 0 on success.
    int setup_code = wiringPiSetupGpio();
    if(setup_code)
    {
        printf("Library failed... Exiting\n");
        return setup_code;
    }
    
    return 0;
}
