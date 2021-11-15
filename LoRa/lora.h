#ifndef LORA_H
#define LORA_H

/*
 * This is an updated library for the REYAX RYLR896. This time, I've decided
 * to write it in C++ for some nice functionality, and hopefully to remove any
 * accidental warnings between languages.
 *
 * Last updated: November 14, 2021
 *
 */


#include <pigpio.h>

#include <iostream>
#include <string>

/* Could wrap this up in a class for better handling, but why be OOP when not needed? */
static int32_t loraHandle = PI_BAD_HANDLE;
static int8_t loraMode = -1;


struct loraMessage {
  int address;
  int length;
  std::string message;
  int RSSI;
  int SNR;
};


/* Function Prototypes */
int32_t loraInit (std::string serDevice, uint32_t baud);

int32_t loraSend (uint16_t address, std::string& message);
int32_t loraReceive (loraMessage& messageData);

int32_t loraSleep (int8_t mode);
int32_t loraWaitForData (std::string& s, uint32_t maxWaitTime = 1000000);

int32_t loraClose (void);

/*
 * Basic LoRa initialization function. Must be called before using.
 *
 * inputs:
 *   serDevice: serial device to use. Either "/dev/ttyAMA0" or "/dev/ttyS0". 
 *              Disable bluetooth so you can use AMA0 since mini UART sucks.
 *
 *   baud: baud rate to use. Use 115200 as that's the default for the RYLR896.
 *
 * outputs:
 *   returns the handle (>=0) if everything went well. <0 indicates failure.
 */
int32_t loraInit (std::string serDevice, uint32_t baud)
{
  if (loraHandle >= 0) return loraHandle;

  int temp = gpioInitialise();
  if (temp < 0) return temp;

  int serFlags = 0; // pigpio doesn't use serFlags yet.
  loraHandle = serOpen(const_cast<char*>(serDevice.c_str()), baud, serFlags);
  if (loraHandle < 0) return loraHandle;

  // Connection established, let's test the LoRa module.
  char testcmd[] = "AT\r\n"; // Just to supress the warning.
  temp = serWrite(loraHandle, testcmd, 4);
  if (temp < 0) return temp;

  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;

  if(s.find("+OK\r\n") == std::string::npos) return -1;

  // We're ready to go, let's get started.
  loraMode = 0;
  return loraHandle;  
}

/*
 * This function sends a message to the specified address.
 *
 * inputs:
 *   address: the address to send to. Address 0 sends to all addresses from [0,65535].
 *   message: string to send. Must have a length less than 240.
 * 
 * outputs:
 *   < 0 indicates failure.
 */
int32_t loraSend (uint16_t address, std::string& message)
{
  if (loraHandle < 0) return loraHandle; // not initialized
  if (loraMode == 1) return PI_BAD_MODE; // sleeping
  if (message.length() > 240) return PI_BAD_PARAM;

  std::string toSend = "AT+SEND=" + std::to_string(address) + "," + std::to_string(message.length()) + "," + message + "\r\n";

  int temp = serWrite(loraHandle, const_cast<char*>(toSend.c_str()), toSend.length());
  if (temp < 0) return temp;

  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;

  if(s.find("+OK\r\n") == std::string::npos) return -1;

  return 0;
}

/*
 * NEW AND IMPROVED RECEIVE FUNCTION! Give it a loraMessage reference, and it
 * fills it with the necessary data. It does all the hard stuff for you!
 *
 * inputs:
 *   messageData: loraMessage struct (see above). Contains Address, Length of message,
 *                the message itself, RSSI, and SNR values.
 *
 * outputs:
 *   < 0 indicates failure.
 *   = 0 indicates no received message.
 *   > 0 indicates message received.
 *
 * WARNING: Do not call this function as rapidly as possible, as junk data will
 *          be in the buffer while the serial line is communicating. I don't 
 *          know why, blame the OS. That being said, do not call this too 
 *          infrequently either, as that will also have negative consequences.
 *          I also noticed that if you give the SEND command a bad length value,
 *          it will send that to this receive. If that happens, and stoi trys to
 *          convert incorrectly, it will throw an error (signal 6)!
 *
 */
int32_t loraReceive (loraMessage& messageData) 
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  int strDataLength = serDataAvailable(loraHandle);

  if (strDataLength > 0)
  {
    char strData[strDataLength];
    int temp = serRead(loraHandle, strData, strDataLength);
    if (temp < 0) return temp;

    std::string s(strData);

    // Got something, let's make sure it's what we want.
    int found = s.find("+RCV=");
    if(found >= 0)
    {
      int equalSign = s.find('=', found + 3);
      int firstComma = s.find(',', equalSign + 1);
      int secondComma = s.find(',', firstComma + 1);

      std::string sAddress = s.substr(equalSign + 1, firstComma - equalSign - 1);
      std::string sLength = s.substr(firstComma + 1, secondComma - firstComma - 1);

      int nLength = std::stoi(sLength);

      std::string sMessage = s.substr(secondComma + 1, nLength);

      int secondLastComma = s.find(',', secondComma + nLength + 1);
      int lastComma = s.find(',', secondLastComma + 1);
      int cr = s.find('\r', lastComma + 1);

      std::string sRSSI = s.substr(secondLastComma + 1, lastComma - secondLastComma - 1);
      std::string sSNR = s.substr(lastComma + 1, cr - lastComma - 1);

      // Parsing done, let's give it back!
      messageData.address = std::stoi(sAddress);
      messageData.length  = nLength;
      messageData.message = sMessage;
      messageData.RSSI    = std::stoi(sRSSI);
      messageData.SNR     = std::stoi(sSNR);

      return strDataLength;
    }
    // If it wasn't there, purge the message, as it wasn't handled adequately.
    // It probably is a bad idea, but whatever.
    // std::cout << s << std::endl;
  }
  else if (strDataLength < 0)
  {
    return strDataLength; 
  }
 
  return 0;
}

/*
 * This function tells the RYLR896 to go into sleep mode or awaken from
 * sleep mode. For some reason, when trying to go into or out of sleep mode,
 * the damn thing likes to spew garbage out onto the serial line, usually
 * just a few newline characters, but still annoying either way. This iteration
 * of the sleep funciton should be able to account for that, as long as whenever
 * you put it into sleep mode, do not access anything until you exit sleep mode.
 * I've added logic to the other functions that do not do anything if it is in
 * sleep mode.
 *
 * intpus:
 *   mode: 1 to put the device into sleep mode. Anything else puts it in 
 *         Transmit and Receive mode (default).
 *
 */
int32_t loraSleep (int8_t mode)
{
  if (loraHandle < 0) return loraHandle; // Not initialized.

  // No need to put it into sleep if it's already asleep.
  uint8_t truemode = (mode == 1) ? 1 : 0;
  if (loraMode == truemode) return loraMode;

  // Wake it up, if it isn't already.
  char testcmd[] = "AT\r\n";
  int temp = serWrite(loraHandle, testcmd, 4);
  if (temp < 0) return temp;

  // Don't care about what's in there, so get rid of it.
  std::string s = "";
  loraWaitForData(s);

  if (truemode)
  {
    char sleep[] = "AT+MODE=1\r\n";
    temp = serWrite(loraHandle, sleep, 11);
    if (temp < 0) return temp;

    loraWaitForData(s);

    if(s.find("+OK\r\n") == std::string::npos) return -1;    
  }
  else
  {
    // We're trying to wake it up, so let's send the test 
    // command a few times to clear the buffer out.
    bool wakingUp = true;
    while (wakingUp)
    {
      serWrite(loraHandle, testcmd, 4);
      loraWaitForData(s);
      if(s.find("+OK\r\n") != std::string::npos) 
        wakingUp = false;
    }
  }

  loraMode = truemode;
  return loraMode;
}


/*
 * This function will wait for some data on the serial line. If it is there,
 * it will read all of it and place it in the reference of the string input.
 * It is currently configured that after waiting one second, it will return
 * anywas as to avoid an infinite loop. Either way, do not call this function
 * unless you are expecting an immediate response back.
 *
 * inputs:
 *   s: the string reference to place the bytes from the serial line.
 *   maxWaitTime: the maximum amount of time to wait in microseconds. Defaults to 1 sec.
 *
 * outpus:
 *   <=0 indicates failure
 *   ==0 indicates timeout
 *   >=0 indicates success, and amount of characters extracted from serial line.
 */
int32_t loraWaitForData(std::string& s, uint32_t maxWaitTime)
{
  uint8_t strLen = 0;
  uint32_t startTick, endTick;
  uint32_t diffTick;

  startTick = gpioTick();
  while(!(strLen = serDataAvailable(loraHandle)))
  {
    endTick = gpioTick();
    diffTick = endTick - startTick;
    // No response after so long, return.
    if(diffTick >= maxWaitTime) return 0;
  }

  char str [strLen];
  int temp = serRead(loraHandle, str, strLen);
  if (temp < 0) return temp;

  s = std::string(str);
  return strLen;
}

/*
 * This funciton closes the connection to the RYLR896. It should be used when
 * finished using the device, say prior to shutdown or escape. It is not
 * critical, but it is good form. 
 *
 * DOES NOT TERMINATE pigpio, do that at the end of your program.
 *
 * outputs:
 *   0 if OK, otherwise PI_BAD_HANDLE (Not initialized)
 */
int32_t loraClose (void)
{
  int oldHandle = loraHandle;
  loraHandle = PI_BAD_HANDLE;
  loraMode = -1;
  return serClose(oldHandle);
}


#endif
