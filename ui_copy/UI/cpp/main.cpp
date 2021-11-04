#include "Ui.h"
#include <math.h>
#include <stdlib.h>     //exit()
#include <stdio.h>

#include "lora.h"

int main(int argc, char *argv[])
{
    LCD_INIT();
    DEV_Delay_ms(1000);

    // Lora init
    int loraTemp = loraInit("/dev/ttyAMA0", 115200);
    if (loraTemp < 0)
    {
      return 0;
    }

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
    //texts.push_back("never say die.");

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

    printf("\nLora stuff\n");

    const char *texts0 = texts[0].c_str();
    loraSend(0, texts[0].length(), (char*) texts0);

    loraMessage loraReceiveTest0;


    for(int seconds = 0; seconds < 10; seconds++)
    {
      loraTemp = loraReceive(&loraReceiveTest0);
      if(loraTemp)
      {
        texts.push_back(loraReceiveTest0.message);
      }
      DEV_Delay_ms(1000);
    }

    LCD_text(texts);
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

    loraClose();
    gpioTerminate();
    return 0;
}
