/*
 * Simple test suite for the LoRa module.
 *
 * to compile: g++ -Wall -pthread -lpigpio -lrt lora_test.cpp -o YOUR_EXE
 *             sudo ./YOUR_EXE
 *
 * Last Updated: November 14, 2021
 *
 */


#include "lora.h"
#include <iostream>
#include <string>

int testWait(void);
int testAddress(void);
int testNetworkID(void);
int testSend(void);
//int testReceive(void);
//int testSleep(void);

int exitTest(int val);

int main(void)
{
  std::cout << "Testing the LoRa module:" << std::endl;
  int handle = loraInit("/dev/ttyAMA0", 115200);
  if (handle < 0) return exitTest(handle);

  bool tWait = true; 
  bool tAddress = true;
  bool tNetworkID = true;

  bool tSend = true;
  //bool tReceive = true;


  int testCode;
  if (tWait && ((testCode = testWait()) < 0))
  {
    std::cout << "\t\ttestWait Failed with code: " << std::to_string(testCode) << std::endl;
    return exitTest(testCode);
  }

  if (tAddress && ((testCode = testAddress()) < 0))
  {
    std::cout << "\t\ttestAddress Failed with code: " << std::to_string(testCode) << std::endl;
    return exitTest(testCode);
  }

  if (tNetworkID && ((testCode = testNetworkID()) < 0))
  {
    std::cout << "\t\ttestNetworkID Failed with code: " << std::to_string(testCode) << std::endl;
    return exitTest(testCode);
  }

  // Address should be 5, NetworkID should be 0 now.

  if (tSend && ((testCode = testSend()) < 0))
  {
    std::cout << "\t\ttestSend Failed with code: " << std::to_string(testCode) << std::endl;
    return exitTest(testCode);
  }

  return exitTest(0);
}

int testWait(void)
{
  std::cout << "\tTesting WaitForData with no expected response: " << std::endl;
  std::string s = "";
  return loraWaitForData(s);
}

int testAddress(void)
{
  std::cout << "\tTesting Address Manipulation" << std::endl;
  if (loraGetAddress() != 0)
    return loraGetAddress();
  
  uint16_t newAddr = 5;
  int check = loraSetAddress(newAddr);
  if (check != newAddr)
    return check;

  if (loraGetAddress() != newAddr)
    return loraGetAddress();

  return 0;
}

int testNetworkID(void)
{
  std::cout << "\tTesting Network ID Manipulation" << std::endl;

  if (loraGetNetworkID() != 0)
    return loraGetNetworkID();

  int8_t newID = 5;
  int check = loraSetNetworkID(newID);
  if (check != newID)
    return check;

  if (loraGetNetworkID() != newID)
    return loraGetNetworkID();

  newID = 0;
  check = loraSetNetworkID(newID);
  if (check != newID)
    return check;

  return 0;
}

int testSend(void)
{
  std::cout << "\tTesting Sending" << std::endl;

  int temp = loraSend(0, "1: HELLO!");
  if (temp < 0) return temp;

  gpioDelay(50000);

  temp = loraSend(5, "2: 50 ms later...");
  if (temp < 0) return temp;

  gpioDelay(5000);

  return 0;
}
//int testReceive(void);

int exitTest(int val)
{
  std::cout << "Closing the LoRa module with value: " << std::to_string(val) << std::endl;
  loraClose();
  gpioTerminate();
  return val;
}
