/* To use pigpio, run: gcc -Wall -pthread -lpigpio -lrt lora_test.c -o lora */
#include<pigpio.h>

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>

#include "lora.h"

void lexit (int);

int main (void)
{
  printf("---Initializing LoRa!---\n");
  int temp = loraInit("/dev/ttyAMA0", 115200);
  if (temp < 0)  lexit(temp);
  
  /*
  printf("---Testing Sleep Mode!---\n");
  temp = loraSleep(1);
  if (temp < 0)
  {
    printf("  Going into sleep failed.\n");
    lexit(temp);
  }

  //gpioDelay(10000);

  temp = loraSleep(1);
  if (temp < 0)
  {
    printf(" Going into sleep twice failed.\n");
    lexit(temp);
  }

  //gpioDelay(10000);

  temp = loraSleep(0);
  if (temp < 0)
  {
    printf("  Waking from sleep failed.\n");
    lexit(temp);
  }
  */
  gpioDelay(10000);
  printf("---Sending message---\n");
  loraSend (5, 5, "HELLO");

  /*
  printf("--- Receiving message---\n");

  uint32_t time = 0;
  loraMessage mymess;
  while(time < 100)
  {
    time++;
    gpioSleep(PI_TIME_RELATIVE, 0, 100000);
    if(loraReceive(&mymess))
    {
      printf("message received: %s\n", mymess.message);
      printf("sender: %d\n", mymess.address);
      printf("length: %d\n", mymess.length);
      printf("RSSI: %d\n", mymess.RSSI);
      printf("SNR: %d\n", mymess.SNR);
    }
  }

  //loraCleanBuffer();
  */
  printf("---Setting Address to 64123---\n");
  temp = loraSetAddress(64123);
  if (temp < 0) 
  {
    lexit (temp);
  }

  printf("---Getting Address of LoRa module!---\n");
  temp = loraGetAddress();
  if (temp < 0)
  {
    lexit (temp);
  }

  printf("---Closing everything up!---\n");
  if (loraClose() < 0)
    printf("  PI_BAD_HANDLE\n");


  printf("---Exiting Normally---\n");
  gpioTerminate();
  return 0;
}

void lexit (int code)
{
  printf("===Exiting program!===\n");
  printf("  error code: %d\n", code);
  loraClose();
  gpioTerminate();
  exit(0);
}
