#ifndef LORA_H
#define LORA_H

/******************************************************************************
 * This is a simple REYAX RYLR896 library. My intention is to make this 
 * library easy to use. It's a work in progress, so if there's anything wrong,
 * please let me know!
 *
 * Random collection of thoughts:
 *  > What happens when there are multiple commands sent, are those saved, or 
 *   are they overwritten? I need to test that.
 *
 *  > If the device is in sleep mode, will it wake up if it is intended to 
 *    receive a message?
 *
 * Last updated: September 25, 2021
 ******************************************************************************/

#include <pigpio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* TODO: remove when not needed anymore: */
#include <stdio.h>

/* Used parameters. Maybe shove them into a struct? Maybe use C++ instead? */
static int32_t loraHandle = PI_BAD_HANDLE;
static int8_t loraMode = -1;

/* Function prototypes */
int32_t loraInit (char *serDevice, uint32_t baud);
int32_t loraSleep (int8_t mode);
int32_t loraSend (uint16_t address, uint8_t payload, char *data);
int32_t loraReceive (char *buffer);
int32_t loraClose (void);

//static void loraWait(uint32_t maxWaitTime);
void loraCleanBuffer (void);


/*
 * Basic LoRa initialization function. Must be called before using.
 *
 * inputs:
 *   serDevice: serial device to use. Either "/dev/ttyAMA0" or "/dev/ttyS0". 
 *     Disable bluetooth so you can use AMA0 since mini UART sucks.
 *
 *   baud: baud rate to use. Use 115200 as that's the default for the RYLR896.
 *
 * outputs:
 *   returns the handle (>=0) if everything went well. <0 indicates failure.
 */
int32_t loraInit (char *serDevice, uint32_t baud)
{
  if (loraHandle >= 0) return loraHandle; 

  int32_t temp = gpioInitialise();
  if (temp < 0) return temp;

  loraHandle = serOpen(serDevice, baud, 0);
  if (loraHandle < 0) return loraHandle;
 
  /* Connection established, testing RYLR896. */
  temp = serWrite(loraHandle, "AT\r\n", 4);
  if (temp < 0) return temp;

  /* This is a busy loop. I'm wondering if there's a better way to do this, *
   * but haven't thought of anything yet. On average, takes 1.4 ms of time  *
   * I'm currently thinking of making a helper function that does the busy  *
   * loop, or if a certain amount of time elapses?                          */
  while(!serDataAvailable(loraHandle)); 

  uint8_t strdataLength = serDataAvailable(loraHandle);
  char strdata [strdataLength];
  temp = serRead(loraHandle, strdata, strdataLength);
  if(temp < 0) return temp;

  /* For some reason, strcmp does not return 0, but strncmp does return 0    *
   * when I expect it to. Could it be becuase of '/0'? I tried extending the *
   * length of strdata, but that didn't seem to help me in this regard.      *
   * What the hell?                                                          */  
  if (strncmp(strdata, "+OK\r\n", strdataLength)) return -1;
 
  /* Everything successful, let's get started. */
  loraMode = 0;
  return loraHandle;  
}

/*
 * This function tells the RYLR896 to go into sleep mode or awaken from
 * sleep mode. This function will clear the serial buffer to ensure it is
 * in sleep mode.
 *
 * inputs:
 *   mode: 1 to put the device in sleep mode. Use anything else to put it in
 *     Transmit and Receive mode (default).
 *
 * outputs:
 *   <0 indicates error.
 */
int32_t loraSleep (int8_t mode)
{
  if (loraHandle < 0) return loraHandle;  

  uint8_t truemode = (mode == 1) ? 1 : 0; 
  if (loraMode == truemode) return loraMode;

  serWrite(loraHandle, "AT\r\n", 4);

  // Possible issue: something already there...
  // Or doesn't get there!
  while(!serDataAvailable(loraHandle)); 

  loraCleanBuffer();

  /* Since the device is now not in Sleep Mode, let's force it if we should */
  if(truemode == 1)
  {
    int temp = serWrite(loraHandle, "AT+MODE=1\r\n", 11);
    if (temp < 0) return temp;

    while(!serDataAvailable(loraHandle));

    uint8_t strdataLength = serDataAvailable(loraHandle);
    char strdata [strdataLength];
    temp = serRead(loraHandle, strdata, strdataLength);
    if(temp < 0) return temp;

    if (strncmp(strdata, "+OK\r\n", strdataLength)) return -1;
  }

  loraMode = truemode;

  return loraMode;
}


/*
 *
 *
 */
int32_t loraSend (uint16_t address, uint8_t payload, char *data)
{ 
  if (loraHandle < 0) return loraHandle;  
  if (loraMode == 1) return PI_BAD_MODE;
  if (payload > 240) return PI_BAD_PARAM;

  uint8_t length = strlen(data);
  if (length > payload) return PI_BAD_PARAM;

  char message [255];

  sprintf(message, "AT+SEND=%d,%d,%s\r\n",address, payload, data);

  printf("message: %s", message);

  int temp = serWrite(loraHandle, message, strlen(message));
  if (temp < 0) return temp;

  while(!serDataAvailable(loraHandle)); 

  uint8_t strdataLength = serDataAvailable(loraHandle);
  char strdata [strdataLength];
  temp = serRead(loraHandle, strdata, strdataLength);
  if(temp < 0) return temp;

  if (strncmp(strdata, "+OK\r\n", strdataLength)) return -1;
  return 0;


}

int32_t loraReceive (char *buffer)
{
  if(serDataAvailable(loraHandle))
  {
    uint8_t strdataLength = serDataAvailable(loraHandle);
    char strdata [strdataLength];
    int temp = serRead(loraHandle, strdata, strdataLength);
    if (temp < 0) return 0;

    memcpy(buffer, strdata, strdataLength);
    loraCleanBuffer();
    return 1;
  }
  else
    return 0;
}

/*
 * This function closes connection with the RYLR896. It should be used when  
 * finished using the device, say on shutdown. It's not critical, but it is
 * good form. Does not terminate pigpio.
 *
 * outputs:
 *   0 if OK, otherwise PI_BAD_HANDLE (Did not initialize)
 */
int32_t loraClose (void)
{
  loraCleanBuffer();
  int temp = serClose(loraHandle);
  loraHandle = PI_BAD_HANDLE;
  loraMode = -1;
  return temp;
}


/*
 * Planned helper function for waiting on expected input. Prevents infinite loops.
 *
 * inputs:
 *   maxWaitTime: Maximum amount of time to wait in microseconds.
 *
static void loraWait(uint32_t maxWaitTime)
{
  printf("    Waiting...\n"); //TODO: Remove eventually
  uint32_t delTime = 0;
  uint32_t startTick = gpioTick();
  //TODO: Getting PI_BAD_PARAM, ser parameter likely. I'm doing something wrong...
  while(!serDataAvailable(loraHandle) && delTime)
  {
    delTime = (gpioTick() - startTick) < maxWaitTime;
  }
}
*/

/*
 * This function clears out the buffer of the RYLR896. Certain functions in
 * this file call this to clear it out to ensure it is in a known state.
 * Some commands insert a random newline just to screw with everything.
 * For some reason, each reply from the module adds it to a list in the buffer
 * and doesn't clear it out. It needs investigation.
 */
void loraCleanBuffer(void)
{
  printf("    cleaning:\n"); //TODO: Remove eventually
  while(serDataAvailable(loraHandle))
  {
    uint8_t len = serDataAvailable(loraHandle);
    char str [len];
    serRead(loraHandle, str, len);
    printf("    ->%s\n", str); //TODO: Remove eventually
  }
}

#endif
