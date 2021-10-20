#include "Ui.h"
#include <math.h>
#include <stdlib.h>     //exit()
#include <stdio.h>

int main(int argc, char *argv[])
{
    LCD_INIT();
    DEV_Delay_ms(1000);

    std::vector<std::string> texts;

    texts.push_back("u1: Hello");
    texts.push_back("u2: Hello");
    texts.push_back("u1: Everything is offline.");
    texts.push_back("u2: Good thing we have emergency radios.");
    texts.push_back("u2: We can at least stay in contact if something goes wrong.");
    texts.push_back("u2: Hello?");
    texts.push_back("you can't see this because it's beyond the screen limit.");
    texts.push_back("Flying");
    texts.push_back("Landing");
    texts.push_back("And");
    texts.push_back("Meanwhile");
    texts.push_back("Initialising");
    texts.push_back("Nonstop");
    texts.push_back("Gnarly");
    texts.push_back("Operations");
    texts.push_back("F.L.A.M.I.N.G.O.s");
    texts.push_back("never say die.");

    LCD_text(texts);
    DEV_Delay_ms(1000);
    LCD_dark_mode();
    DEV_Delay_ms(1000);
    LCD_light_mode();
    DEV_Delay_ms(1000);
    LCD_text_index(8);
    DEV_Delay_ms(1000);
    LCD_battery(15);
    DEV_Delay_ms(1000);
    LCD_battery(50);
    DEV_Delay_ms(1000);
    LCD_set_font(24);
    DEV_Delay_ms(1000);
    LCD_set_font(16);
    DEV_Delay_ms(1000);

    for (size_t i = 0; i<(texts.size() - 8)*2; i++)
    {
    	LCD_up();
    	DEV_Delay_ms(500);
    }

    for (size_t i = 0; i<(texts.size() - 8); i++)
    {
    	LCD_down();
    	DEV_Delay_ms(500);
    }

    LCD_exit();

    return 0;
}
