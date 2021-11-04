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
 * Last updated: October 7, 2021
 ******************************************************************************/
 
 /* To use pigpio, run: gcc -Wall -pthread -lpigpio -lrt YOUR_PROGRAM_HERE.c -o executable_name */

#include <pigpio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* TODO: remove when not needed anymore: */
#include <stdio.h>

/* Used parameters. Maybe shove them into a struct? Maybe use C++ instead? */
static int32_t loraHandle = PI_BAD_HANDLE;
static int8_t loraMode = -1;

/* Structs */
typedef struct {
  uint16_t address;
  uint8_t length;
  char message[255];
  int16_t RSSI;
  int16_t SNR;
} loraMessage;

/* Function prototypes */
int32_t loraInit (char *serDevice, uint32_t baud);
int32_t loraSleep (int8_t mode);

/* Sending/Receiving */
int32_t loraSend (uint16_t address, uint8_t payload, char *data);
int32_t loraReceive (loraMessage *messageData);

/* Setters/Getters */
int32_t loraSetAddress(uint16_t address);
int32_t loraGetAddress(void);

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
 * This function sends a message to the specified address. 
 *
 * inputs:
 *   address: the address to send it to. Address 0 sends to all addresses from [0,65535].
 *   payload: the length of the message to send in bytes. Maximum of 240.
 *   data: message to send, ASCII formatted.
 *
 * outputs:
 *   < 0 indicates failure.
 *
 */
int32_t loraSend (uint16_t address, uint8_t payload, char *data)
{ 
  if (loraHandle < 0) return loraHandle;  
  if (loraMode == 1) return PI_BAD_MODE;
  if (payload > 240) return PI_BAD_PARAM;

  uint8_t length = strlen(data);
  if (length > payload) return PI_BAD_PARAM; // Maybe instead a payload, use length of message ?

  char message [260]; // Guarantees large enough buffer.

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

/*
 * This function provides the user with received messages, if they are available.
 * This should be called frequently enough that a message does not get overwritten
 * and lost. Will clear the buffer after receiving one message.
 *
 * CURRENTLY GETTING SIGNAL 11. I'M NOT SURE WHY, MORE INVESTIGATION IS NEEDED!
 *
 * inputs:
 *   messageData: A loraMessage struct. 
 *
 * outputs:
 *  < 0 indicates failure, 0 indicates no message, > 0 indicates message
 *
 */
int32_t loraReceive (loraMessage *messageData)
{
  if(serDataAvailable(loraHandle))
  {
    uint8_t strdataLength = serDataAvailable(loraHandle);
    char strdata [strdataLength];
    int temp = serRead(loraHandle, strdata, strdataLength);
    if (temp < 0) return temp;

    char strAddr [10];
    char strLength [10];
    char strMessage [260];
    char strRSSI [10];
    char strSNR [10];

    /* Read through the string, and parse only what we want. */
    int i;
    for (i = 0; i < strdataLength; i++)
    {
      /* All messages start with "+RCV=", just skip to the important stuff. */
      if (strdata[i] == 'V')
      { 
        /* Get the sender's address. */
        i+=2;
        int j = 0;
        
        while (strdata[i+j] != ',')
        {
          strAddr[j] = strdata[i+j];
          j++;
        }
        strAddr[j] = '\0';

        /* Get the length of the message */
        i = i+j+1;
        j = 0;

        while (strdata[i+j] != ',')
        {
          strLength[j] = strdata[i+j];
          j++;
        }
        strLength[j] = '\0';

        /* Get the message */
        i = i+j+1;
        temp = atoi(strLength);

        for(j = 0; j < temp; j++)
        {
          strMessage[j] = strdata[i+j]; 
        }
        strMessage[j] = '\0';

        /* Get the RSSI */
        i = i+j+1;
        j = 0;

        while (strdata[i+j] != ',')
        {
          strRSSI[j] = strdata[i+j];
          j++;
        }
        strRSSI[j] = '\0';

        /* Get the SNR */
        i = i+j+1;
        j = 0;

        while (strdata[i+j] != '\r')
        {
          strSNR[j] = strdata[i+j];
          j++;
        }
        strSNR[j] = '\0';

        messageData->address = atoi(strAddr);
        messageData->length = atoi(strLength);
        strcpy(messageData->message, strMessage);
        messageData->RSSI = atoi(strRSSI);
        messageData->SNR = atoi(strSNR);

        //printf("address: %d\n", messageData->address);
        //printf("length: %d\n", messageData->length);
        //printf("message: %s\n", messageData->message);
        //printf("RSSI: %d\n", messageData->RSSI);
        //printf("SNR: %d\n", messageData->SNR);

        break;        
      }
    }

    loraCleanBuffer();
    return 1;
  }
  else
    return 0;
}

/*
 * This function sets the address of this LoRa module. The module will save it
 * in its EEPROM.
 *
 * inputs:
 *
 * outputs:
 *   0 if OK, otherwise failure.
 *
 */
int32_t loraSetAddress(uint16_t address)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  char setAddr [20]; // Bigger than it needs to be.

  sprintf(setAddr, "AT+ADDRESS=%d\r\n",address);

  int temp = serWrite(loraHandle, setAddr, strlen(setAddr));
  if (temp < 0) return temp;

  while(!serDataAvailable(loraHandle));

  uint8_t strdataLength = serDataAvailable(loraHandle);
  char strdata [strdataLength];
  temp = serRead(loraHandle, strdata, strdataLength);
  if(temp < 0) return temp;

  if (strncmp(strdata, "+OK\r\n", strdataLength)) return -1;
  return 0;
}


/*
 * This function gets the address the LoRa Module has saved in its EEPROM.
 *
 * outputs:
 *  <0 indicates failure, otherwise the address of the module, should be a 
 *    number from [0,65535].
 *
 */
int32_t loraGetAddress(void)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  // TODO: Clear the buffer first?

  int temp = serWrite(loraHandle, "AT+ADDRESS?\r\n", 13); 
  if (temp < 0) return temp;

  while(!serDataAvailable(loraHandle));

  uint8_t strdataLength = serDataAvailable(loraHandle);
  char strdata [strdataLength];
  temp = serRead(loraHandle, strdata, strdataLength);
  if (temp < 0) return temp;

  // The message should be "+ADDRESS=#"
  printf(":::Message: %s\n", strdata);
  return -1; // TODO: FIX!
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
