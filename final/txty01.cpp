#include "../LoRa/lora.h"
#include "../UI/cpp/ui/Ui.h"
#include <iostream>

int main(void)
{
  std::cout << "starting.." << std::endl;

  // Initialize LoRa and LCD
  int loraHandle = loraInit("/dev/ttyAMA0", 115200);
  if (loraHandle < 0) return -1;

  LCD_INIT();
  
  std::cout << "Success" << std::endl;

  return 0;
}
