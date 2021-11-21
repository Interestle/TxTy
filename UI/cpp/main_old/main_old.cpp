/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 21, 2021
 */

// How to compile and run:
// cd TxTy/TxTy/UI/cpp
// cp ../../final/txty.cpp main.cpp
// sudo make clean
// sudo make -j 4
// sudo ./main


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
};

deviceSettings currentSettings;

/* Functions */
std::string txtyCommand(std::string& command);

void saveToFile (std::vector<std::string> messagesToSave);
deviceSettings loadFromFile (std::vector<std::string>& messagesToLoad);

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
  int pageUpPin = physPinToGpio(31);
  int pageDownPin = physPinToGpio(33);
  pinMode(pageUpPin, INPUT);
  pinMode(pageDownPin, INPUT);

  // Battery indicator is a single pin on the charger, if it is 0,
  // it's gonna die soon (~3.2 V)
  // Tell the battery is low.
  // int batteryPin = physPinToGpio(TODO:FINDPIN);
  // pinMode(batteryPin, INPUT);

  // Various values needed
  std::vector<std::string> savedMessages;
  std::vector<char> currentMessage;
  int ch;

  bool updateScreen;
  loraMessage fromLora;

  // Load data from text file.
  // deviceSettings savedSettings = load(savedMessages);
  // loraSetAddress(savedSettings.thisAddress);
  // loraSetNetworkID(savedSettings.thisNetworkID);
  // loraSetRFParameter(savedSettings.thisRFParameter);
  // addrToSend = savedSettings.sendAddress);
  // currentFont = savedSettings.font;
  // LCD_set_font(currentFont);
  // displayMode = savedSettings.displayMode;
  // displayMode ? LCD_light_mode() : LCD_dark_mode();

  uint32_t startSaveTick, endSaveTick, startInactiveTick, endInactiveTick;
  startSaveTick = gpioTick();
  startInactiveTick = gpioTick();

  int isInactive = 0;

  const uint32_t fiveMinutes = 1000000 * 60 * 5;
  const uint32_t thirtySeconds = 1000000 * 5;

  while (1)
  {
    // Keyboard exclusive characters
    ch = getch();
    if(ch != -1)
    {
      isInactive = 0;
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

          if ((stringToSend[0] == '!') && (stringToSend.length() > 1))
          {
            // No good way to clear out the screen in function, just do here.
            if(stringToSend == "!clear") savedMessages.clear();
            else if(stringToSend == "!save") saveToFile(savedMessages);
            else if(stringToSend == "!load") 
            {
              deviceSettings test = loadFromFile(savedMessages);

              std::cout << "addr: " << test.thisAddress << std::endl;
              std::cout << "id: " << test.thisNetworkID << std::endl;
              std::cout << "rf: " << test.thisRFParameter << std::endl;
              std::cout << "send: " << test.sendAddress << std::endl;
              std::cout << "font: " << test.font << std::endl;
              std::cout << "disp: " << test.displayMode << std::endl;              
            } 
            else savedMessages.push_back(txtyCommand(stringToSend));
          }
          else
          {
            loraSend(currentSettings.sendAddress, stringToSend);
          
            stringToSend = "U1: " + stringToSend;
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
       savedMessages.push_back("U2: " + fromLora.message);
       updateScreen = true;
    }
    
    // Additional buttons 
    pageUp = (pageUp << 1) | digitalRead(pageUpPin); 
    pageDown = (pageDown << 1) | digitalRead(pageDownPin);

    if(pageUp == 0x3F)
    {
      LCD_up();
      isInactive = 0;
    }

    if(pageDown == 0x3F)
    {
      LCD_down();
      isInactive = 0;
    }

    // Battery Indicator 
    //if(!digitalRead(batteryPin))
    //  LCD_battery(20);
    //  save();

    // Save messages every 5 minutes
    endSaveTick = gpioTick();
    if(endSaveTick - startSaveTick >= fiveMinutes)
    {
      startSaveTick = endSaveTick;
      saveToFile(savedMessages);
    }

    // Turn display off every 30 seconds of inactivity
    endInactiveTick = gpioTick();
    if(endInactiveTick - startInactiveTick >= thirtySeconds)
    {
      std::cout << "Going into sleep mode!" << std::endl;
      isInactive = 1;
      pwmWrite(LCD_BL, 0);
    }

    if(!isInactive)
    {
      std::cout << "Out of sleep mode!" << std::endl;
      pwmWrite(LCD_BL, 512);
      startInactiveTick = gpioTick();
      isInactive = 1;
    }


    // Update LCD

    if(updateScreen)
    {
      updateScreen = false;
      LCD_messages(savedMessages);
    }    
  }
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
      addrMessage = "Address set to: " + std::to_string(addrParam);

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
      idMessage = "Network ID set to: " + std::to_string(idParam);

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

  else if(command.find("!sendto:") == 0) // MAKE SURE PARENTHESIS ARE WHERE THEY SHOULD BE!
  {
    // Force compliance.
    currentSettings.sendAddress = parameter & 0xFFFF;
    return "Sending to:" + std::to_string(parameter & 0xFFFF) + ".";
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
  
  // TODO: else if(command.find("!load") == 0) {}
  
  // TODO: else if(command.find("!shutdown") == 0) {}
  
  // TODO: Implement a sleep function? I don't think it's worth it.
  // TODO: Implement the AT+CPIN password of the network?
  
  
  // I just wanted a silly command that isn't documented.
  else if(command.find("!why") == 0)
  { // type 'why' into MATLAB.
    return txtyWhy();
  }

  else if(command.find("!help") == 0)
  { // The LCD currently does NOT like newlines!
    return "Available commands: !addr:#, !id:#, !dark, !light, !sendto:#, !font:#, !range, !clear, !save, !load, !shutdown, !help";
  }

  return "invalid command: " + command;
}

/*
 *
 */
void saveToFile (std::vector<std::string> messagesToSave)
{
  std::ofstream txtySave ("txtySave.txt");
  if(txtySave.is_open())
  {
    txtySave << currentSettings.thisAddress << "\n";
    txtySave << currentSettings.thisNetworkID << "\n";
    txtySave << currentSettings.thisRFParameter << "\n";
    txtySave << currentSettings.sendAddress << "\n";
    txtySave << currentSettings.font << "\n";
    txtySave << currentSettings.displayMode << "\n";

    for(size_t i = 0; i < messagesToSave.size(); i++)
    {
      txtySave << messagesToSave[i] << "\n";
    }
  }
  txtySave.close();
}

/*
 *
 *
 */
deviceSettings loadFromFile (std::vector<std::string>& messagesToLoad)
{
  std::vector<std::string> readData;
  deviceSettings settings;

  std::ifstream txtyRead ("txtySave.txt");
  if(txtyRead.is_open())
  {
    txtyRead >> settings.thisAddress;
    txtyRead >> settings.thisNetworkID;
    txtyRead >> settings.thisRFParameter;
    txtyRead >> settings.sendAddress;
    txtyRead >> settings.font;
    txtyRead >> settings.displayMode;
    
    std::string savedString;
    while(getline(txtyRead,savedString))
    {
      readData.push_back(savedString);
    }
  }
  txtyRead.close();

  messagesToLoad = readData;
  return settings;
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
 *
 */
void txtyExit(int exitCode)
{
  endwin();
  loraClose();
  gpioTerminate();
  LCD_exit();
  exit(exitCode);
}
