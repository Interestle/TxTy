/* This file represents the final project for the TxTy Project at the 
 * University of Utah for the fall semester of 2021.
 *
 * The team is comprised of Colton Watson, Benjamin Leaprot, Phelan Hobbs, and Seth Jackson
 *
 * Last updated: November 6, 2021
 */

#include "../LoRa/lora.h"

// TODO: Need to figure out how to properly include the UI:
//#include "../UI/cpp/ui/Ui.h"

#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

int main(void)
{
  std::cout << "starting.." << std::endl;

  // Initialize LoRa and LCD
  int loraHandle = loraInit("/dev/ttyAMA0", 115200);
  if (loraHandle < 0) return -1;

  //LCD_INIT();
  vector<string> savedMessages;


  while (1)
  {

  }

  return 0;
}
