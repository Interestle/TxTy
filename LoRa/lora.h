#ifndef LORA_H
#define LORA_H

/*
 * This is an updated library for the REYAX RYLR896. This time, I've decided
 * to write it in C++ for some nice functionality, and hopefully to remove any
 * accidental warnings between languages.
 *
 * Last updated: November 17, 2021
 *
 */


#include <pigpio.h>

#include <iostream>
#include <string>

#define LORA_RF_S 0xA717
#define LORA_RF_L 0xC417

/* Could wrap this up in a class for better handling, but why be OOP when not needed? */
static int32_t loraHandle = PI_BAD_HANDLE;
static int8_t loraMode = -1;
static uint16_t loraAddress = 0xBEEF;
static int8_t loraNetworkID = -1;
static uint16_t loraRFParameter = 0;

/* Simple struct to use for receiving messages. */
struct loraMessage {
  int address;
  int length;
  std::string message;
  int RSSI;
  int SNR;
};


/* Function Prototypes */
int32_t loraInit (std::string serDevice, uint32_t baud);

int32_t loraSend (uint16_t address, std::string message);
int32_t loraReceive (loraMessage& messageData);

int32_t loraSetAddress (uint16_t addr);
int32_t loraGetAddress (void);

int32_t loraSetNetworkID (int8_t id);
int32_t loraGetNetworkID (void);

int32_t loraSetRFParameter (uint16_t parameters);
int32_t loraGetRFParameter (void);

// Implement the AT+CPIN command? I don't really like it
// int32_t loraSetNetworkPassword (std::string password);

int32_t loraSleep (int8_t mode);
int32_t loraWaitForData (std::string& s, uint32_t maxWaitTime = 1000000);

int32_t loraClose (void);

/*
 * Basic LoRa initialization function. Must be called before using. 
 * Initializes the device with an address of 0 (it receives all messages).
 * Initializes with Network ID of 0 (public)
 *
 * inputs:
 *   serDevice: serial device to use. Either "/dev/ttyAMA0" or "/dev/ttyS0". 
 *              Disable Bluetooth so you can use AMA0 since mini UART sucks.
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
  char testcmd[] = "AT\r\n"; // Just to suppress the warning.
  temp = serWrite(loraHandle, testcmd, 4);
  if (temp < 0) return temp;

  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;

  if(s.find("+OK\r\n") == std::string::npos) return -1;

  // Can communicate, set default Address, Network ID, and RF parameters.
  temp = loraSetAddress(0);
  if (temp < 0) return temp;

  temp = loraSetNetworkID(0);
  if (temp < 0) return temp;

  temp = loraSetRFParameter(LORA_RF_S);
  if (temp < 0) return temp;

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
int32_t loraSend (uint16_t address, std::string message)
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

  if(s.find("+OK") < 0) return -1;

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
 *          it will send that to this receive. If that happens, and stoi tries 
 *          to convert incorrectly, it will throw an error (signal 6)!
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
 * This function sets the address that the LoRa module will use. The address
 * is used to identify the transmitter or receiver in a message.
 *
 * inputs:
 *   addr: unsigned 16-bit integer representing all possible addresses to use.
 *
 * outputs:
 *   < 0 indicates failure.
 *
 */
int32_t loraSetAddress (uint16_t addr) 
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;
  if (loraAddress == addr) return loraAddress;
  
  std::string toSend = "AT+ADDRESS=" + std::to_string(addr) + "\r\n";

  int temp = serWrite(loraHandle, const_cast<char*>(toSend.c_str()), toSend.length());
  if (temp < 0) return temp;
 
  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;
 
  if(s.find("+OK\r\n") < 0) return -1;

  // Everything successful, let's keep track of changes.
  loraAddress = addr;
  
  return loraAddress;  
}

/*
 * This function returns the current address of the device that it is using.
 *
 * outputs: < 0 indicates failure, >= 0 indicates current address being used.
 */
int32_t loraGetAddress (void)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;
  
  // The old way took advantage of asking the module directly. That sucked,
  // so instead, let's just keep track of the address locally. It just means
  // we should set it upon initialization as well.
  return loraAddress;
}
 
/*
 * This function sets the ID of the LoRa network. This is a group function. 
 * Only modules connected to the same nework ID can communicate with each other.
 *
 * inputs:
 *   id: integer value ranging from 0-15. Network 0 is the public ID of LoRa.
 *
 * outputs: < 0 indicates failure.
 *
 */
int32_t loraSetNetworkID (int8_t id)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;
  if (id < 0) return id;
  if (loraNetworkID == id) return loraNetworkID;

  // Force good values.
  int8_t trueID = id & 0xF;

  std::string toSend = "AT+NETWORKID=" + std::to_string(trueID) + "\r\n";

  int temp = serWrite(loraHandle, const_cast<char*>(toSend.c_str()), toSend.length());
  if (temp < 0) return temp;

  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;

  if (s.find("+OK\r\n") < 0) return -1;

  // Everything successful, save changes.
  loraNetworkID = trueID;

  return loraNetworkID;
}

/*
 * This function retrieves the Network ID used by the LoRa module.
 *
 * outputs:
 *   < 0 indicates failure
 *  >= 0 indicates Network ID.
 */
int32_t loraGetNetworkID (void)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  return loraNetworkID;
}

/*
 * This function sets the RF parameters of the RYLR896. The intention behind 
 * it is that we can these parameters however we want. However, I don't feel
 * like spending the time and getting it perfect. Instead, the input can 
 * either be LORA_RF_S or LORA_RF_L.
 *
 * inputs:
 *   parameters: these are the parameters to send to the LoRa module. There are
 *     four parameters, each less than the value of 15, so each nybble of the
 *     parameters input corresponds to these parameters. They go as following:
 *     0x[4][3][2][1]
 *     [4] Spreading Factor. The larger, better sensitivity, but slower. [7-12]
 *     [3] Bandwidth. Smaller bandwidth means more sensitive, but slower.[ 0-9]
 *     [2] Coding Rate. ??? Always set to 1.                             [ 1-4]
 *     [1] Preamble code. Bigger loses less data.                        [ 4-7]
 *
 * outputs: <0 indicates failure.
 *
 */
int32_t loraSetRFParameter (uint16_t parameters)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  // Get all of the parameters, convert them to their characters, and make the command.
  // char spreadingFactor = ((parameters & 0xF000) >> 12);
  // char bandwidth       = ((parameters & 0x0F00) >>  8);
  // char codingRate      = ((parameters & 0x00F0) >>  4);
  // char preamble        =  (parameters & 0x000F); 
  // std::string toSend = "AT+PARAMETER=" + /*converted parameters*/ + "\r\n";


  // Could do all of that above, but let's just enforce good input.
  std::string toSend;

  if (parameters == LORA_RF_S)
    toSend = "AT+PARAMETER=10,7,1,7\r\n";
  else if (parameters == LORA_RF_L)
    toSend = "AT+PARAMETER=12,4,1,7\r\n";
  else
    return PI_BAD_PARAM;

  // Send to LoRa.
  int temp = serWrite(loraHandle, const_cast<char*>(toSend.c_str()), toSend.length());
  if (temp < 0) return temp;

  // Wait for response back.
  std::string s = "";
  temp = loraWaitForData(s);
  if (temp < 0) return temp;

  if (s.find("+OK\r\n") < 0) return -1;

  loraRFParameter = parameters;
  return loraRFParameter;
}

/*
 * This function retrieves the RF parameters used by the LoRa module.
 * Devices using different RF parameters can not communicate!
 *
 * outputs:
 *   <= 0 indicates failure, otherwise the value should either be LORA_RF_S or
 *       LORA_RF_L!
 *
 */
int32_t loraGetRFParameter (void)
{
  if (loraHandle < 0) return loraHandle;
  if (loraMode == 1) return PI_BAD_MODE;

  return loraRFParameter;
}

/*
 * Here is just a little prototype of a function to set the password. It 
 * currently isn't implemented, and don't expect it to be right now. I don't
 * really know how it works on the RYLR896, and I don't feel like learning it.
 * The command is 'AT+CPIN=<Password>', where <Password> is a 32 character
 * long password from 000...001 to FFF...FFF. Only by using the same password
 * can devices communicate. "After resetting, the previously password will 
 * disappear." Whatever that means. Software reset? Hardware reset? Reset CMD?
 * Does that mean to set the password to no password, set password = 0?
 * I don't know. It's not documented well, and hard to understand.
 * Maybe something to test out later.
 * /
int32_t loraSetNetworkPassword (std::string password)
{
  int maxLength = 16;
  if (password.length() > maxLength) return -1;
 
  // Create a full password 
  // WARNING: NOT HOW YOU SHOULD EVER CREATE PROPER PASSWORDS!
  char cPassword[maxLength];
  for (int i = 0; i < maxLength; i++)
	truePassword[i] = ((maxLength - i) > password.length()) ? '0' : password[(maxLength - (i + password.length()-1))]; 	  

  // Now just get the hex values of that string.
  char const hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  
  std::string sPassword = "";
  for (int i = 0; i < maxLength; i++)
  {
    sPassword += hex_chars[(cPassword & 0xF0) >> 4 ];
	sPassword += hex_chars[(cPassword & 0x0F)];
  }
  
  // Should now have a 32 character long string of hex values,
  // now we just need to send it to the module.
  
  std::string toSend = "AT+CPIN=" + sPassword + "\r\n";
  
  // Send to Lora
  // Wait for response back.
  return 0; 
}

*/ // loraSetNetworkPassword (fix function header comment)

/*
 * This function tells the RYLR896 to go into sleep mode or awaken from
 * sleep mode. For some reason, when trying to go into or out of sleep mode,
 * the damn thing likes to spew garbage out onto the serial line, usually
 * just a few newline characters, but still annoying either way. This iteration
 * of the sleep function should be able to account for that, as long as whenever
 * you put it into sleep mode, do not access anything until you exit sleep mode.
 * I've added logic to the other functions that do not do anything if it is in
 * sleep mode.
 *
 * inputs:
 *   mode: 1 to put the device into sleep mode. Anything else puts it in 
 *         Transmit and Receive mode (default).
 *
 * outputs:
 *   < 0 indicates failure, otherwise it returns its current mode.
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
 * anyways as to avoid an infinite loop. Either way, do not call this function
 * unless you are expecting an immediate response back.
 *
 * inputs:
 *   s: the string reference to place the bytes from the serial line.
 *   maxWaitTime: the maximum amount of time to wait in microseconds. Defaults to 1 sec (could probably make much smaller).
 *
 * outputs:
 *   < 0 indicates failure
 *   = 0 indicates timeout
 *   > 0 indicates success, and amount of characters extracted from serial line.
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

  if (strLen < 0) return strLen;

  char str [strLen];
  int temp = serRead(loraHandle, str, strLen);
  if (temp < 0) return temp;

  s = std::string(str);
  return strLen;
}

/*
 * This function closes the connection to the RYLR896. It should be used when
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
  loraAddress = 0xBEEF;
  loraNetworkID = -1;
  return serClose(oldHandle);
}


#endif
