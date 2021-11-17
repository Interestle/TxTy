/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 15, 2021
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
#include <string>
#include <vector>
#include <exception>
#include <ncurses.h>

std::string txtyCommand(std::string& command);
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

  // int networkID = loraGetNetworkID();
  // int currentAddress = loraGetAddress();

  // What I'm thinking for init, if you want to change it.

  // Then we'll just need a couple of functions to update the value
  // Something like, obviously change the function names to whatever you want:
  // LCD_NETWORKID(loraGetNetworkID());
  // LCD_LORA_ADDRESS(loraGetAddress());

  
  LCD_INIT();

  // Initialize additional buttons and controls
  uint8_t pageUp, pageDown;
  int pageUpPin = physPinToGpio(31);
  int pageDownPin = physPinToGpio(33);
  pinMode(pageUpPin, INPUT);
  pinMode(pageDownPin, INPUT);

  // Various values needed
  std::vector<std::string> savedMessages;
  std::vector<char> currentMessage;
  int ch;

  bool updateScreen;
  loraMessage fromLora;

  while (1)
  {
    // Keyboard exlcusive characters
    ch = getch();
    if(ch != -1)
    {
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
            if(stringToSend == "!clear") savedMessages.clear();
            else savedMessages.push_back(txtyCommand(stringToSend));
          }
          else
          {
            int addrToSend = 0;
            loraSend(addrToSend, stringToSend);
          
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
        LCD_up();

    if(pageDown == 0x3F)
        LCD_down();

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
    if (loraSetAddress(addrParam) < 0)
      return "ERROR: FAILED TO SET ADDRESS?";
    else
      return "Address set to: " + std::to_string(addrParam);

    // TODO: Tell LCD that the address changed?
  }

  else if(command.find("!id:") == 0)
  {
    // Set Network ID this device will use.
    // Force good range of values. Could check parameter instead.
    int8_t idParam = parameter & 0x0F; 
    if (loraSetNetworkID(idParam) < 0)
      return "ERROR: FAILED TO SET NETWORK ID?";  
    else
      return "Network ID set to: " + std::to_string(idParam);

    // TODO: Tell LCD that the id changed?
  }

  else if(command.find("!dark") == 0)
  {
    LCD_dark_mode();
    return "Going into dark mode!";
  }

  else if(command.find("!light") == 0)
  {
    LCD_light_mode();
    return "Going into light mode!";
  }

/*
  // TODO: These commands
  else if(command.find("!sendto:" == 0))
  {

  }

  else if(command.find("!font:" == 0))
  {
    // make sure font is reasonable size
    //LCD_set_font(size);
  }

  else if(command.find("!range" == 0))
  {
    // NOT implemented yet.

  }
*/

  // I just wanted a silly command that isn't documented.
  else if(command.find("!why") == 0)
  { // type 'why' into MATLAB. This is the first one I got:
    return "For the love of Pete.";  
  }
  
  else if(command.find("!help") == 0)
  { // The LCD currently does NOT like newlines!
    return "Avalable commands: !addr:#, !id:#, !dark, !light, !sendto:#, !font:#, !range, !clear, !help";
  }

  return "invalid command: " + command;
}

/*
 * I called the why command in MATLAB a few times. Here are a few bangers, or
 * a few I thought were good enough to add here. 
 */
std::string txtyWhy(void)
{
  switch(gpioTick() & 0xF)
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
    case 0x9: return "I dunno!";
    case 0xA: return "Joe knew it was a good idea.";
    case 0xB: return "For her approval.";
    case 0xC: return "Ask again later.";
    case 0xD: return "Don't you have something better to do?";
    case 0xE: return "Don't ask.";
    case 0xF: return "How should I know?";
  }

  // Shouldn't get here, but have a return anyways.
  return "";
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