/* To use pigpio, run: gcc -Wall -pthread -lpigpio -lrt lora.c -o lora */

#include<pigpio.h>

#include<stdio.h>
#include<stdint.h>
#include<string.h>

int main (void)
{
  printf("---Initizing pigpio!---\n");

  if (gpioInitialise() < 0)
  {
    printf("pigpio initilization failed.\n");
    return -1;
  }
  
  // Open serial...
  printf("---Testing Serial Now!---\n");
  
  int serHandle = serOpen("/dev/ttyAMA0", 115200, 0);
  if (serHandle < 0)
  {
    printf("  pigpio Serial Open failed...\n");
    
    if(serHandle == PI_SER_OPEN_FAILED)
      printf("  Can't open serial device.\n");
    
    if(serHandle == PI_NO_HANDLE)
      printf("  No handle available.\n");
      
    return serHandle;
  }
 
  char* strTest = "AT\r\n";
  serWrite(serHandle, strTest, strlen(strTest));

  while(serDataAvailable(serHandle) == 0);

  int dataAvail = serDataAvailable(serHandle);
  printf("dataAvail: %d\n", dataAvail);
  if (dataAvail)
  {
    char strdata [dataAvail];
    if(serRead(serHandle, strdata, dataAvail) < 0)
    {
      // Find out if PI_BAD_HANDLE, PI_BAD_PARAM, or PI_SER_READ_NO_DATA
      return -1;
    }

    printf(strdata);

    
  



  }








  printf("---Closing everything up---\n");
  int serC = serClose(serHandle);
  if (serC < 0)
  {
    printf("  PI_BAD_HANDLE");
    gpioTerminate();
    return serC;
  }
  gpioTerminate();
  return 0;
}
