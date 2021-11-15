
#include "lora.h"
#include <iostream>
#include <string>

int main(void)
{
  std::cout << "Testing the LoRa module:" << std::endl;
  int handle = loraInit("/dev/ttyAMA0", 115200);
  std::cout << "\tInitialized with handle: " + std::to_string(handle) << std::endl;
 
  std::cout << "\tTesting WaitForData with no expected response: " << std::endl;
  std::string s = "";
  loraWaitForData(s);
/*
  std::cout << "\tTesting Sleep:" << std::endl;
  loraSleep(0);
  loraSleep(1);
  gpioSleep(PI_TIME_RELATIVE, 5, 0);
  loraSleep(0);

  std::cout << "\tTesting Sending:" << std::endl;
  loraSend(5, "Hello!");
  gpioSleep(PI_TIME_RELATIVE, 1, 0);
  loraSend(5, "Testing!");
*/
  std::cout << "\tTesting Receiving:" << std::endl;

  uint32_t startTick, endTick;
  uint32_t diffTick;

  loraMessage messy;

  startTick = gpioTick();
  while(diffTick < 30000000)
  {
    if(loraReceive(messy) > 0)
    {
      std::cout << "ADDR: " << messy.address << std::endl;
      std::cout << "LEN: "  << messy.length << std::endl;
      std::cout << "MESS: " << messy.message << std::endl;
      std::cout << "RSSI: " << messy.RSSI << std::endl;
      std::cout << "SNR: "  << messy.SNR << std::endl;
    }
    gpioDelay(500000);
    endTick = gpioTick();
    diffTick = endTick - startTick;
  }
  

  std::cout << "Closing the LoRa module:" << std::endl;
  loraClose();
  gpioTerminate(); // Close gpio library.
  return 0;
}
