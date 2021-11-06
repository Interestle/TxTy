/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 6, 2021
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
  clear();

  // Initialize LoRa and LCD
  int loraHandle = loraInit("/dev/ttyAMA0", 115200);
  if (loraHandle < 0) return -1;

  LCD_INIT();


  std::vector<std::string> savedMessages;
  std::vector<char> currentMessage;
  char ch;

  while (1)
  {
    // Keyboard exlcusive characters
    ch = getch();
    if(ch)
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
        }
      }    
      else if(ch == '\n')
      {

      }
      else
      {
        currentMessage.push_back(ch);
      }
      std::cout << std::string(currentMessage.begin(), currentMessage.end()) << std::endl;
    }

    // LoRa messages

    // Additional buttons (?)

    // Update LCD


    
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
