/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: December 5, 2021
 */


#include "/home/pi/TxTy/TxTy/LoRa/lora.h"
#include "Ui.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <ncurses.h>

struct deviceSettings
{
  int thisAddress;
  int thisNetworkID;
  int thisRFParameter;
  int sendAddress;
  int font;
  int displayMode;
  int displayBrightness;
} currentSettings;


/* Functions */
std::string txtyCommand(std::string& command);

void saveToFile (std::vector<std::string>& messagesToSave);
void loadFromFile (std::vector<std::string>& messagesToLoad);

std::string txtyWhy(void);
void txtyExit(int exitCode);


int main(void)
{
  // ncurses init
  initscr();
  cbreak();
  noecho();
  timeout(20);
  clear();

  // Initialize LoRa and LCD
  int loraHandle = loraInit("/dev/ttyAMA0", 115200);
  if (loraHandle < 0) return -1;
  
  LCD_INIT();

  // Initialize additional buttons and controls
  uint8_t pageUp, pageDown;
  pageUp = pageDown = 0xFF;
  int pageUpPin = physPinToGpio(38);
  int pageDownPin = physPinToGpio(40);
  pinMode(pageUpPin, INPUT);
  pinMode(pageDownPin, INPUT);

  // Battery indicator is a single pin on the charger, if it is 0,
  // it's gonna die soon (~3.2 V)
  // Tell the battery is low.
  int batteryPin = physPinToGpio(33);
  pinMode(batteryPin, INPUT);

  // Notification LED
  int ledPin = physPinToGpio(7);
  pinMode(ledPin, OUTPUT);

  // Various values needed
  std::vector<std::string> savedMessages;
  std::vector<char> currentMessage;
  int ch;

  bool updateScreen;
  loraMessage fromLora;

  bool buttonPushed = false;
  bool isScreenOn = true;

  // Load data from text file.
  loadFromFile(savedMessages);

  // Various timers.
  uint32_t startSaveTick, endSaveTick; 
  uint32_t startTimeoutTick, endTimeoutTick;
  startSaveTick = gpioTick();
  startTimeoutTick = startSaveTick;

  const int32_t fiveMinutes  = 1000000 * 60 * 5;
  int32_t timeoutTimer = (1000000 * 30) | 0x1; // Off by default.
  while (1)
  {
    // Keyboard exclusive characters
    ch = getch();
    if(ch != -1)
    {
      buttonPushed = true;
      if(ch == 27) // Esc
      {
        txtyExit(0);
      }
      else if(ch == KEY_BACKSPACE || ch == KEY_DC || ch == 127)
      {
        if(currentMessage.size() != 0)
        {
          currentMessage.pop_back();
          std::string s(currentMessage.begin(), currentMessage.end());
          LCD_message_box(s);
        }
      }    
      else if(ch == '\n')
      {
        if(currentMessage.size() != 0 && currentMessage.size() < 241)
        {
          std::string stringToSend(currentMessage.begin(), currentMessage.end());

          // If the message is a command, do that instead of sending it.
          if ((stringToSend[0] == '!') && (stringToSend.length() > 1))
          {
            // Commands that need to modify savedMessages.
            if(stringToSend == "!clear") savedMessages.clear();
            else if(stringToSend == "!save") saveToFile(savedMessages);
            else if(stringToSend == "!load") loadFromFile(savedMessages);
            else if(stringToSend == "!timeout")
            {
              timeoutTimer ^= 0x1;
              std::string temp = (timeoutTimer & 0x1) ? "Screen will not time out" : "Screen will now time out";
              savedMessages.push_back(temp);
            }
            else if(stringToSend == "!shutdown")
            { 
              saveToFile(savedMessages);
              txtyExit(0);
            }
            // Every other command.
            else savedMessages.push_back(txtyCommand(stringToSend));
          }
          else
          {
            loraSend(currentSettings.sendAddress, stringToSend);
          
            stringToSend = "U" + std::to_string(currentSettings.thisAddress) + ": " + stringToSend;
            savedMessages.push_back(stringToSend);
          }
          currentMessage.clear();
          LCD_message_box("");

          updateScreen = true;
        }            
      }
      else
      {
        currentMessage.push_back(ch);

        std::string s(currentMessage.begin(), currentMessage.end());
        LCD_message_box(s);
      }
    } 

    // LoRa messages
    if(loraReceive(fromLora) > 0)
    {
       savedMessages.push_back("U" + std::to_string(fromLora.address) + ": " + fromLora.message);
       updateScreen = true;
       digitalWrite(ledPin, HIGH);
       //TODO: check RSSI/SNR, suggest user switch range if poor or weak?
    }
    
    // Additional buttons 
    pageUp = (pageUp << 1) | digitalRead(pageUpPin); 
    pageDown = (pageDown << 1) | digitalRead(pageDownPin);


    if(!digitalRead(pageUpPin))//if(pageUp == 0xF0)
    {
      std::cout << "UP!" << std::endl;
      LCD_up();
      buttonPushed = true;
    }

    if(!digitalRead(pageDownPin))//if(pageDown == 0xf0)
    {
      std::cout << "DOWN" << std::endl;
      LCD_down();
      buttonPushed = true;
    }

    // Battery Indicator 
    if(!digitalRead(batteryPin))
    {
      LCD_battery(20);
      saveToFile(savedMessages);
    }

    // Save messages every 5 minutes
    endSaveTick = gpioTick();
    if(endSaveTick - startSaveTick >= fiveMinutes)
    {
      startSaveTick = endSaveTick;
      saveToFile(savedMessages);
    }

    // Turn off display after so much time of inactivity.
    endTimeoutTick = gpioTick();
    int diffTimeoutTick = endTimeoutTick - startTimeoutTick;
    if(isScreenOn && !(timeoutTimer & 0x1) && (diffTimeoutTick  >= timeoutTimer))
    {
      pwmWrite(LCD_BL, 0);
      isScreenOn = false;
    }

    // Action performed, reset timer
    if(buttonPushed)
    {
      startTimeoutTick = gpioTick();
      digitalWrite(ledPin, LOW);
 
      // If the screen is currently off, turn it on.
      if(!isScreenOn)
      {
        pwmWrite(LCD_BL, currentSettings.displayBrightness);
        isScreenOn = true;
      }
      buttonPushed = false;
    }


    // Update LCD
    if(updateScreen)
    {
      updateScreen = false;
      LCD_messages(savedMessages);
    }    
  }

  // End of loop. Shouldn't get here.
  return 0;
}

/*
 * This function is a collection of commands that TxTy understands. All 
 * commands begin with a '!' character, and all commands that need an integer
 * parameter can only take a single integer proceeding a single colon character.
 *
 * inputs: 
 *   command: a string that starts with a '!' (bang)
 *
 * outputs: a string indicating completion or failure.
 */
std::string txtyCommand(std::string& command)
{
  int parameter = 0;
  std::size_t found = command.find(':', 1);

  if(found != std::string::npos)
  {
    try
    {
      parameter = std::stoi(command.substr(found + 1)); // Things might go bad here
    }
    catch(std::exception& e)
    {
      return "Invalid command: " + command;
    }
  }

  // Could try mapping strings to int then doing a cool case statement with some enum,
  // but that's dev time, instead, since there's only going to be a few commands,
  // let's use a bunch of conditional checks.
  if(command.find("!addr:") == 0)
  {
    // Set the address of this device.
    // Force compliance. Could check parameter, but this works.
    uint16_t addrParam = parameter & 0xFFFF;
    std::string addrMessage;

    if (loraSetAddress(addrParam) < 0)
      addrMessage = "ERROR: FAILED TO SET ADDRESS?";
    else
      addrMessage = "This address is now: " + std::to_string(addrParam);

    currentSettings.thisAddress = loraGetAddress();
    LCD_LORA_ADDRESS(loraGetAddress());
    return addrMessage;
  }

  else if(command.find("!id:") == 0)
  {
    // Set Network ID this device will use.
    // Force good range of values. Could check parameter instead.
    int8_t idParam = parameter & 0x0F;
    std::string idMessage;
    if (loraSetNetworkID(idParam) < 0)
      idMessage = "ERROR: FAILED TO SET ID?";
    else
      idMessage = "This network ID is: " + std::to_string(idParam);

    currentSettings.thisNetworkID = loraGetNetworkID();
    LCD_NETWORKID(loraGetNetworkID());
    return idMessage;
  }

  else if(command.find("!dark") == 0)
  {
    LCD_dark_mode();
    currentSettings.displayMode = 0;
    return "Going into dark mode!";
  }

  else if(command.find("!light") == 0)
  {
    LCD_light_mode();
    currentSettings.displayMode = 1;
    return "Going into light mode!";
  }

  else if(command.find("!sendto:") == 0)
  {
    // Force compliance.
    currentSettings.sendAddress = parameter & 0x0FFFF;
    
    std::string val;
    if(currentSettings.sendAddress)
      val = "Sending to: " + std::to_string(currentSettings.sendAddress);
    else
      val = "Sending to everyone!";
    return val;
  }

  else if(command.find("!font:") == 0)
  {
    // make sure font is reasonable size

    int goodFonts[] = {8,12,16,20,24};

    for(int i = 0; i < 5; i++)
    {
      if(parameter == goodFonts[i])
      {
        LCD_set_font(parameter);
        currentSettings.font = parameter;
        return "Setting font to " + std::to_string(parameter);
      }     
    }

    return "Unable to set font size. Please use 8, 12, 16, 20, or 24.";
  }

  else if(command.find("!range") == 0)
  {
    int currentParam = loraGetRFParameter();
    if (currentParam < 0) return "ERROR: Using incorrect RF parameter?";

    uint16_t nNextRange; 
    std::string sNextRange;
    
    if (currentParam == LORA_RF_S)
    {
      nNextRange = LORA_RF_L;
      sNextRange = "long";
    }
    else
    {
      nNextRange = LORA_RF_S;
      sNextRange = "short";
    }

    int temp = loraSetRFParameter(nNextRange);
    if (temp < 0) return "ERROR: RF FAILED to set. Code: " + std::to_string(temp) + ".";

    currentSettings.thisRFParameter = loraGetRFParameter();
    return "Now in " + sNextRange + " range. Make sure others are in the same range mode!";
  }
 
  else if(command.find("!bright:") == 0)
  {
    if (parameter % 5)
      return "Brightness % must be a multiple of five!";

    if ((parameter > 100) || (parameter < 5))  
      return "Brightness must be between 5 and 100!";

    currentSettings.displayBrightness = (512 * parameter) / 100;
    pwmWrite(LCD_BL, currentSettings.displayBrightness);

    return "Display brightness is now at: " + std::to_string(parameter) + "%";
  }
  
  // TODO: Implement a sleep function? I don't think it's worth it.
  // TODO: Implement the AT+CPIN password of the network?
  
  
  // I just wanted a silly command that isn't documented.
  else if(command.find("!why") == 0)
  { // type 'why' into MATLAB.
    return txtyWhy();
  }

  else if(command.find("!help") == 0)
  { // The LCD currently does NOT like newlines!
    return "Available commands: !addr:#, !id:#, !dark, !light, !sendto:#, !font:#, !range, !bright:#, !timeout, !clear, !save, !load, !shutdown, !help";
  }

  return "invalid command: " + command;
}

/*
 * This function saves the currently used settings of the application,
 * as well as the list of sent and received messages to txtySave.txt.
 */
void saveToFile (std::vector<std::string>& messagesToSave)
{
  std::ofstream txtySave ("txtySave.txt");
  if(txtySave.is_open())
  {
    txtySave << currentSettings.thisAddress << " ";
    txtySave << currentSettings.thisNetworkID << " ";
    txtySave << currentSettings.thisRFParameter << " ";
    txtySave << currentSettings.sendAddress << " ";
    txtySave << currentSettings.font << " ";
    txtySave << currentSettings.displayMode << " ";
    txtySave << currentSettings.displayBrightness << std::endl;

    for(size_t i = 0; i < messagesToSave.size(); i++)
    { // Only save messages that are actually messages.
      if(messagesToSave[i][0] == 'U')
        txtySave << messagesToSave[i] << std::endl;
    }
  }
  txtySave.close();
}


/*
 * This function loads settings for the application to use as well as the saved
 * list of sent and received messages.
 */
void loadFromFile (std::vector<std::string>& messagesToLoad)
{
  std::vector<std::string> readData;

  std::ifstream txtyRead ("txtySave.txt");
  if(txtyRead.is_open())
  {
    txtyRead >> currentSettings.thisAddress;
    txtyRead >> currentSettings.thisNetworkID;
    txtyRead >> currentSettings.thisRFParameter;
    txtyRead >> currentSettings.sendAddress;
    txtyRead >> currentSettings.font;
    txtyRead >> currentSettings.displayMode;
    txtyRead >> currentSettings.displayBrightness;
    
    std::string savedString;
    while(std::getline(txtyRead, savedString))
    {
      readData.push_back(savedString);
    }
  }
  txtyRead.close();

  // Now that everything is read, let's update LoRa and LCD.
  int temp = loraSetAddress(currentSettings.thisAddress);
  if (temp < 0) txtyExit(temp);
  temp = loraSetNetworkID(currentSettings.thisNetworkID);
  if (temp < 0) txtyExit(temp);
  temp = loraSetRFParameter(currentSettings.thisRFParameter);
  if (temp < 0) txtyExit(temp);

  LCD_LORA_ADDRESS(loraGetAddress());
  LCD_NETWORKID(loraGetNetworkID());
  
  LCD_set_font(currentSettings.font);
  currentSettings.displayMode ? LCD_light_mode() : LCD_dark_mode();
  pwmWrite(LCD_BL, currentSettings.displayBrightness);

  messagesToLoad = readData;
  LCD_messages(messagesToLoad);
}



/*
 * I called the why command in MATLAB a few times. Here are a few of my
 * favorites or a few I thought were good enough to add here. 
 */
std::string txtyWhy(void)
{ 
  switch(gpioTick() & 0xF) // Super good RNG
  {
    case 0x0: return "For the love of Pete.";
    case 0x1: return "A not excessively tall and not very good engineer knew it was a good idea.";
    case 0x2: return "To satisfy a rich and rich engineer.";
    case 0x3: return "He told me to.";
    case 0x4: return "I wanted it that way.";
    case 0x5: return "You wanted it.";
    case 0x6: return "Some terrified hamster suggested it.";
    case 0x7: return "Why not?";
    case 0x8: return "Can you rephrase that?";
    case 0x9: return "I dunno.";
    case 0xA: return "Joe knew it was a good idea.";
    case 0xB: return "For her approval.";
    case 0xC: return "Ask again later.";
    case 0xD: return "Don't you have something better to do?";
    case 0xE: return "Don't ask.";
    case 0xF: return "How should I know?";
  }

  // Shouldn't get here, but have a return anyways.
  return "?";
}


/*
 * Bail function. Call this upon exit.
 */
void txtyExit(int exitCode)
{ 
  pwmWrite(LCD_BL, currentSettings.displayBrightness);
  endwin();
  loraClose();
  gpioTerminate();
  LCD_exit();
  exit(exitCode);
}
