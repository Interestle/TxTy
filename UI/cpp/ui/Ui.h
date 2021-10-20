/*****************************************************************************
* | File      	:   Ui.h
* | Author      :   Seth Jackson   
* | Info        : This is a basic UI using waveshare's example code for their
* 		  2in LCD Module.
*
*----------------
* |	This version:   V1.0
* | Date        :   2021-09-26
* | Info        :   Basic version
*
******************************************************************************/

#ifndef _UI_H_
#define _UI_H_

#include "DEV_Config.h"
#include "LCD_2inch.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include <vector>
#include <string>

void LCD_INIT(void);
void LCD_text(std::vector<std::string> &texts);
void LCD_set_font(int size); 
void LCD_battery(int charge);
void LCD_up(void);
void LCD_text_index(int text_index);
void LCD_down(void);
void LCD_dark_mode();
void LCD_light_mode();
void LCD_refresh(void);
void LCD_exit(void);

#endif
