/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 11, 2021
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
#include <ncurses.h>


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

  uint8_t pageUp, pageDown;
  int pageUpPin = physPinToGpio(31);
  int pageDownPin = physPinToGpio(33);
  pinMode(pageUpPin, INPUT);
  pinMode(pageDownPin, INPUT);


  std::vector<std::string> savedMessages;
  std::vector<char> currentMessage;
  int ch;

  bool updateScreen;
  char fromLora[255] = "";

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
          
          int addrToSend = 0;
          loraSend(addrToSend, stringToSend.size(), (char*) stringToSend.c_str());
          
          stringToSend = "U1: " + stringToSend;
          savedMessages.push_back(stringToSend);
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
      std::string loraString(fromLora);
      
      std::cout << loraString << std::endl;

      int found = loraString.find("+RCV=");

      if(found >= 0)
      {

       int equalSign = loraString.find('=');
       int firstComma = loraString.find(',');
       int secondComma = loraString.find(',', firstComma + 1);


       //printw("1st: %i\n", firstComma);
       //printw("2nd: %i\n", secondComma);

       std::string mAS = loraString.substr(equalSign + 1, firstComma - equalSign - 1);
       std::string mLS = loraString.substr(firstComma + 1, secondComma - firstComma - 1);

       int messageAddr = std::stoi(mAS);
       int messageLength = std::stoi(mLS);
      
       std::string messageData = loraString.substr(secondComma + 1, messageLength);
       //printw("addr: %i\n", messageAddr);
       //printw("length: %i\n", messageLength);
       //std::cout << "MESSAGE: " + messageData << std::endl;

       savedMessages.push_back("U2: " + messageData);
       updateScreen = true;
      }

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


void txtyExit(int exitCode)
{
  endwin();
  loraClose();
  LCD_exit();
  exit(exitCode);
}
