/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 10, 2021
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
          
          int addrToSend = 0;
          loraSend(addrToSend, stringToSend.size(), (char*) stringToSend.c_str());
          
          stringToSend = "U1: " + stringToSend;
          savedMessages.push_back(stringToSend);
          currentMessage.clear();


          std::cout << std::endl;
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
    if(loraReceive(&fromLora) > 0)
    {
      std::string receivedMessage = fromLora.message;

      savedMessages.push_back("U2: " + receivedMessage);
      updateScreen = true; 
    }

    // Additional buttons (?)

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
