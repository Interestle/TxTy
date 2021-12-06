/*****************************************************************************
* | File      	:   Ui.h
* | Author      :   Seth Jackson   
* | Info       	: Basic UI using waveshare's example code for the their 2in
*		  LCD module. It uses 8 fuctions starting with LCD_INIT and 
*		  LCD_exit both need to be used. 
*
*		  The LCD_INIT sets up everything needed to start painting but malloc's a pointer that needs
*		  to be freed in LCD_exit.
* 
*		  LCD_messages receives a vector of strings and prints each string on it's own line. 
*
*		  LCD_message_box can be used to display the current message being typed. 
*		  The message will be printed in the box at the bottom of the screen. 
*
*		  LCD_set_font takes an int representing
*		  the font size and will change the text size displayed on the UI
*		  unless it is not a size I have built in. In that case it will 
*		  be set to 16. 
*
*		  LCD_battery takes an int representing the 
*		  percentage of charge left. If the percentage is 20 or less
*		  it will change the color of the battery icon to red. 
*
*		  The LCD_up
*		  fuction will move the text up one and the LCD_down moves the text
*		  down one. 
*
*		  LCD_select_message_by_index will jump to a specific text based on index.
*		  LCD_dark_mode will change the background to darker colors
*		  and LCD_light_mode changes the background to lighter colors.
*		  
*		  LCD_NETWORKID will change the displayed network id at the top left of the screen.
*		  and LCD_LORA_ADDRESS will change the address displayed to the right of it.
*
*		  LCD_refresh is mostly just used for the other fuctions
*		  to display the changes.
*
*----------------
* |	This version:   V2.3
* | Date        :   2021-12-6
* | Info        :
*
******************************************************************************/


#ifndef _UI_H_
#define _UI_H_

#include "GUI_BMP.h"
#include <vector>
#include <string>
#include <stdio.h>	//printf()
#include <stdlib.h>	//exit()
#include <signal.h>     //signal()
#include <math.h>

void LCD_INIT(void);
void LCD_messages(std::vector<std::string> &new_messages);
void LCD_message_box(std::string text);
void LCD_set_font(int size); 
void LCD_battery(int charge);
void LCD_up(void);
void LCD_select_message_by_index(int message_index);
void LCD_down(void);
void LCD_dark_mode();
void LCD_light_mode();
void LCD_NETWORKID(int new_netId);
void LCD_LORA_ADDRESS(int new_address);
void LCD_refresh(void);
void LCD_exit(void);

UWORD *BlackImage;
std::string netId = "0";
std::string address = "0";
std::vector<std::string> messages;
int battery = 305;
UWORD background_color = WHITE;
UWORD draw_color = BLACK;
UWORD battery_color = GREEN;
sFONT font = Font16;
int message_box_top = 240 - (font.Height + 10);	
std::vector<std::string> message_box_text;
std::string current_message_box = " ";
unsigned int text_limit = 24; // used to help wrap text around screen.
size_t starting_message = 0; // Index used to select which message is read first. It then prints only the messages after this message.
double row_num = 0; // used to count all rows. This is needed for page label math.
double page_size = 0; // used to store the amount of rows that can fit on the screen.


/*
 * Basic INIT for the 2inch LCD
 */
void LCD_INIT(void)
{
    	// Exception handling:ctrl + c
    	signal(SIGINT, Handler_2IN_LCD);
    
    	/* Module Init */
	if(DEV_ModuleInit() != 0)
	{
        	DEV_ModuleExit();
        	exit(0);
    	}
	
    	/* LCD Init */
	printf("2inch LCD demo...\r\n");
	LCD_2IN_Init();
	LCD_2IN_Clear(background_color);
	LCD_SetBacklight(1010);
	
    	UDOUBLE Imagesize = LCD_2IN_HEIGHT*LCD_2IN_WIDTH*2;
    	if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) 
	{
        	printf("Failed to apply for black memory...\r\n");
        	exit(0);
    	}
	
    	// /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    	Paint_NewImage(BlackImage, LCD_2IN_WIDTH, LCD_2IN_HEIGHT, 90, WHITE, 16);
    	Paint_Clear(background_color);
	Paint_SetRotate(ROTATE_270);

	// /* GUI */
    	printf("drawing...\r\n");
	
	Paint_Clear(BLACK);
	for (int j = 61; j < 170; j += 5)
	{
		// Draw first t.
		int i = 0;
		for (i = 60; i < j; i++)
		{
			Paint_DrawLine(75, (i+10), 85, i, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		for (i = 45; i < (j-60); i++)
		{
			Paint_DrawLine(i, 100, (i+10), 110, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		// Draw x.
		for (i = 0; i < (j-110); i++)
		{
			Paint_DrawLine((105+i), (120+i), (115+i), (120+i), GREEN, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
			Paint_DrawLine((165-i), (120+i), (155-i), (120+i), GREEN, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		// Draw second t.
		for (i = 60; i < j; i++)
		{
			Paint_DrawLine(185, (i+10), 195, i, YELLOW, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		for (i = 155; i < (j+50); i++)
		{
			Paint_DrawLine(i, 100, (i+10), 110, YELLOW, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		// Draw y.
		for (i = 0; i < (j-125); i++)
		{
			Paint_DrawLine((210+i), (80+i), (220+i), (80+i), BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}
	
		for (i = 0; i < (j-120); i++)
		{
			Paint_DrawLine((280-i), (80+i*2), (270-i), (80+i*2), BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		}

		LCD_2IN_Display((UBYTE *)BlackImage);
	}
	Paint_Clear(WHITE);
	
	// Draw radio wave with circles.
	Paint_DrawCircle(250, 120, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawCircle(250, 120, 40, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawCircle(250, 120, 60, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

	// Draw first t.
	int i = 0;
	for (i = 60; i < 170; i++)
	{
		Paint_DrawLine(75, (i+10), 85, i, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	for (i = 45; i < 110; i++)
	{
		Paint_DrawLine(i, 100, (i+10), 110, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	// Draw x.
	for (i = 0; i < 60; i++)
	{
		Paint_DrawLine((105+i), (120+i), (115+i), (120+i), BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
		Paint_DrawLine((165-i), (120+i), (155-i), (120+i), BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	// Draw second t.
	for (i = 60; i < 170; i++)
	{
		Paint_DrawLine(185, (i+10), 195, i, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	for (i = 155; i < 220; i++)
	{
		Paint_DrawLine(i, 100, (i+10), 110, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	// Draw y.
	for (i = 0; i < 40; i++)
	{
		Paint_DrawLine((210+i), (80+i), (220+i), (80+i), BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}
	
	for (i = 0; i < 50; i++)
	{
		Paint_DrawLine((280-i), (80+i*2), (270-i), (80+i*2), BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
	}

	LCD_2IN_Display((UBYTE *)BlackImage);

	DEV_Delay_ms(3000);

	Paint_Clear(background_color);
    	
	// /*2.Drawing on the image*/
	
	message_box_text.push_back(" ");
	LCD_refresh();	
    
}

/*
 * Set font of text.
 * text_limit is used to wrap the text around before it reaches the scroll bar.
 * each text_limit was counted by printing to screen and counting every character that fits on the screen.
 */
void LCD_set_font(int size)
{
	if (size == 8)
	{
		font = Font8;
		text_limit = 53;
	}
	else if (size == 12)
	{
		font = Font12;
		text_limit = 38;
	}
	else if (size == 16)
	{
		font = Font16;
		text_limit = 24;
	}	
	else if (size == 20)
	{
		font = Font20;
		text_limit = 20;
	}
	else if (size == 24)
	{
		font = Font24;
		text_limit = 16;
	}	
	else
	{
		printf("bad font size. Set to Font 16.\r\n");
		font = Font16;
		text_limit = 24;	
	}

	LCD_message_box(current_message_box);	
	LCD_refresh();	

}

/*
 * Sets text printed to the UI.
 * Takes a vector of strings then clears the current text before
 * inserting the strings from the vector.
 */
void LCD_messages(std::vector<std::string> &new_messages)
{
	messages.clear();
	messages.insert(messages.end(), new_messages.begin(), new_messages.end());
	LCD_refresh();
//	printf("page length %f\n", page_size);
//	printf("number of rows %f\n", row_num);
}

void LCD_message_box(std::string text)
{
	current_message_box = text;
	message_box_text.clear();
	message_box_top = 240 - (font.Height + 10);
	std::string next_line_of_text;
	
	// if the text is larger then the text_limit of the font then it needs to be split into substings smaller then the text_limit.
	while (text.length() > (text_limit - 2))
	{
		std::size_t space = text.find_last_of(" ", text_limit); // find the first space before the text_limit.

		if ((space == std::string::npos) || (space == 0)) // if there is no space or only one at the beginning.
		{
			space = text_limit - 1;
			next_line_of_text = text.substr(0,space);
			
			message_box_top -= font.Height;
			if(message_box_top < 30 + font.Height + 10)
			{
				message_box_top += font.Height;
				text.erase(0,text.length()-1);
				text.append(" ...");
				break;
			}
			
			message_box_text.push_back(next_line_of_text);
			text.erase(0,space);
		}
		else // if there is a space between words.
		{			
			next_line_of_text = text.substr(0,space);
			
			message_box_top -= font.Height;

			message_box_text.push_back(next_line_of_text);
			text.erase(0,space);
		}
	}
	
	message_box_text.push_back(text);
	LCD_refresh();
}

/*
 * Sets the battery icon.
 * takes an int representing the percentage of charge.
 * if charge is equal or less then 20 then the color
 * is changed to RED.
 */
void LCD_battery(int charge)
{
		
	battery = ((30*charge)/100) + 275;

	if ((charge > 20) && (100 >= charge))
	{
		battery_color = GREEN;
	}
	else if ((20 >= charge) && (charge > 0))
	{
		battery_color = RED;
	}
	else if (charge == 0)
	{
		battery_color = BLACK;
	}
	else
	{
		printf("Not a valid charge.\r\n");
		free(BlackImage);
    		BlackImage = NULL;
		exit(1);
	}

	LCD_refresh();

}
void LCD_select_message_by_index(int message_index)
{
	if (0 >= message_index)
	{
		starting_message = 0;
	}
	else if (message_index < static_cast<int>(messages.size()))
	{
		starting_message = message_index;
	}
	else
	{
		starting_message = messages.size() - 1;
	}

	LCD_refresh();
}
/*
 * Changes UI to move up in the text.
 * if already at the first text it won't do anything.
 */
void LCD_up(void)
{
	starting_message--;

	if ((0 > starting_message) || (messages.size() == 0))
	{
		starting_message = 0;
	}
	else if (starting_message >= messages.size())
	{
		starting_message = messages.size() - 1;
	}

	LCD_refresh();

}

/*
 * Changes UI to move down in the text.
 * if already at the last text it won't do anything.
 */
void LCD_down(void)
{
	starting_message++;
	
	if ((0 > starting_message) || (messages.size() == 0))
	{
		starting_message = 0;
	}
	else if (starting_message >= messages.size())
	{
		starting_message = messages.size() - 1;
	}
	
	LCD_refresh();

}

/*
 * activate dark mode
 */
void LCD_dark_mode()
{
	background_color = BLACK;
	draw_color = WHITE;
	LCD_refresh();
}

/*
 * return to normal mode
 */
void LCD_light_mode()
{
	background_color = WHITE;
	draw_color = BLACK;
	LCD_refresh();
}

/*
 * changes displayed network ID.
 */
void LCD_NETWORKID(int new_netId)
{
	netId = std::to_string(new_netId);
	LCD_refresh();
}

/*
 * changes displayed address.
 */
void LCD_LORA_ADDRESS(int new_address)
{
	address = std::to_string(new_address);
	LCD_refresh();
}

/*
 * Refresh screen.
 */
void LCD_refresh(void)
{
	Paint_Clear(background_color); // clear screen.

	int row = 35; // used to messure where to print text to screen.
	double starting_row; // used to find first line that is actually printed to the screen.
	int row_starting_from_zero = 35; /* this is also counting to where to print to the screen but starting from zero
				  this is use to mesure how many rows fits on the screen.*/
	row_num = 0; // used to count all rows. This is needed for page label math.
	page_size = 0; // used to store the amount of rows that can fit on the screen.

	for (size_t i = 0; i<messages.size(); i++)
	{
		std::string current_message = messages[i];
		char* next_message;		

		/* when i is equal to the start index text can start being printed and starting_row is equal to
		the number of rows that have been counted.*/
		if (i == starting_message)
		{
			starting_row = row_num;
		}
	
		// if the text is larger then the text_limit of the font then it needs to be split into substings smaller then the text_limit.
		while (current_message.length() > text_limit)
		{
			std::size_t space = current_message.find_last_of(" ", text_limit); // find the first space before the text_limit.
			std::string sub_message;
	
			if ((space == std::string::npos) || (space == 0)) // if there is no space or only one at the beginning.
			{
				space = text_limit - 1;
				sub_message = current_message.substr(0,space);

				/* if the next row will go past the edge of the screen then I want to remove the last four characters in the substring
				 and replace it with three dots*/
				if((row + font.Height + 10) > (message_box_top - font.Height))
				{
					sub_message.erase(sub_message.length()-5,sub_message.length()-1);
					sub_message.append("...");
				}

				current_message.erase(0,space);

			}
			else // if there is a space between words.
			{
				
				sub_message = current_message.substr(0,space);

				/* if the next row will go past the edge of the screen then I want to remove the last word in the substring
				 and replace it with three dots*/
				if((row + font.Height + 10) > (message_box_top - font.Height))
				{
					std::size_t sub_space = sub_message.find_last_of(" ");
					sub_message.erase(sub_space+1,current_message.length()-1);
					sub_message.append("...");
				}

				current_message.erase(0,space);
				
				// delete the space from the split.	
				if (current_message.length() > 3)
				{
					current_message.erase(0,1);
				}

			}
			
			
			// if the next row will print of the screen then we have counted all the rows that fit on the screen.
			if((row_starting_from_zero > (message_box_top - font.Height)) && (page_size == 0))
			{
				page_size = row_num;
			}
	

			// provided it won't print off the screen and we reached the start index we print to screen.
			if ((row < (message_box_top - font.Height)) && (i >= starting_message))
			{
				next_message = const_cast<char*>(sub_message.c_str());
				Paint_DrawString_EN(0, row, next_message, &font, background_color, draw_color);

				row += (font.Height + 10);
			}
				
			row_starting_from_zero += (font.Height + 10);
			row_num++;	
		}	
		
		// if the next row will print of the screen then we have counted all the rows that fit on the screen.
		if((row_starting_from_zero > (message_box_top - font.Height)) && (page_size == 0))
		{
			page_size = row_num;
		}	

		// provided it won't print off the screen and we reached the start index we print to screen.
		if ((row < (message_box_top - font.Height)) && (i >= starting_message))
		{
			next_message = const_cast<char*>(current_message.c_str());
			Paint_DrawString_EN(0, row, next_message, &font, background_color, draw_color);
	
			row += (font.Height + 10);
		}
		
		row_starting_from_zero += (font.Height + 10);
		row_num++;
	}

	std::string num_pages;
	std::string current_page;
	std::string page;
	char* page_label;	

	if ((row_num > 0) && (page_size > 0))
	{
		num_pages = std::to_string((int)(round(row_num/page_size))); /* get number of pages by dividing number of all rows divided by how many rows fit on a page then add one.
										I don't know why the plus one is needed but the divison will round down*/
		current_page = std::to_string((int)(floor(starting_row/page_size)) + 1); /* get current page by dividing number of rows before start divided by how many rows fit 
											on a page then add one*/
		page = current_page + "/" + num_pages;
		page_label = const_cast<char*>(page.c_str());
 
	}
	else
	{
		page = "1/1";
		page_label = const_cast<char*>(page.c_str());
	}

	int scroll_bar_start = 45;	
	int scroll_bar_end = 225;
	
	if ((row_num > 0) && (page_size > 0))
	{
		scroll_bar_end = 225 - (row_num*5);
		if (scroll_bar_end > 45)
		{
			/* provided there is text and the limit of the scroll bar was not reached. 
			The end of the scroll bar is decided by the number of rows plus the start of the scroll bar. */
			
			scroll_bar_start = 45 + (starting_row*5);
		}
		else
		{
			/* If there is to many texts then the limit could be reached.
			When this happens I divide the rest of the bar by number of rows
			times the number of rows before the start row. */

			scroll_bar_start = 45 + (int)(round((180/row_num)*starting_row));
		}

		scroll_bar_end = 230 - (row_num*5) + (scroll_bar_start - 45);
	}

	// make sure the scroll bar start and end stay 5 pixel away from each other and doesn't pass the edges of the scroll bar.		
	if (scroll_bar_start < 45)
	{
		scroll_bar_start = 45;
	}
 
	if (scroll_bar_end < (scroll_bar_start + 5))
	{
		scroll_bar_end = scroll_bar_start + 5;
	} 
	
	if (scroll_bar_end > 225)
	{
		scroll_bar_end = 225;
	}	

	if (scroll_bar_start > (scroll_bar_end - 5))
	{
		scroll_bar_start = scroll_bar_end - 5;
	}
	
	// network ID label
	std::string netIdLabel = "ID: " + netId;
	
	Paint_DrawRectangle(0, 0, 100, 30, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(10, 10, const_cast<char*>(netIdLabel.c_str()), &Font16,  background_color, draw_color);	
	
	// Address label
	Paint_DrawRectangle(100, 0, 200, 30, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(110, 10, const_cast<char*>(address.c_str()), &Font16, background_color, draw_color);	

	// Page label
	Paint_DrawRectangle(200, 0, 260, 30, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(210, 10, page_label, &Font16, background_color, draw_color);	

	// Battery
	Paint_DrawRectangle(260, 0, 320, 30, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(274, 9, 306, 26, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(275, 10, battery, 26, battery_color, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawRectangle(306, 15, 308, 20, draw_color, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	// up and down labels
	Paint_DrawRectangle(290, 30, 320, 135, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(305, 80, 300, 90, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(305, 80, 310, 90, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawRectangle(290, 135, 320, 240, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(305, 195, 300, 185, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(305, 195, 310, 185, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

	// scroll bar
	Paint_DrawRectangle(270, 30, 290, 240, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(280, 35, 277, 40, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(280, 35, 283, 40, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawRectangle(272, scroll_bar_start, 288, scroll_bar_end, GRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawLine(280, 235, 277, 230, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(280, 235, 283, 230, draw_color, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

        // message box
	Paint_DrawRectangle(0, message_box_top, 270, 240, draw_color, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	
	char* next_line;
	int line_placement = 5;
	for (size_t i = 0; i<message_box_text.size(); i++)
	{
		std::string message_box_line = message_box_text[i];
		next_line = const_cast<char*>(message_box_line.c_str());
		Paint_DrawString_EN(0, (message_box_top + line_placement), next_line, &font,  background_color, draw_color);
		line_placement += font.Height;	
	}

	// /*3.Refresh the picture in RAM to LCD*/
    	LCD_2IN_Display((UBYTE *)BlackImage);

}
/*
 * Fuction needed to free BlackImage pointer before exiting.
 */
void LCD_exit(void)
{
	/* Module Exit */
	Paint_Clear(BLACK);
	Paint_DrawRectangle(10, 10, 310, 230, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(15, 15, 305, 225, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(25, 40, "Now shutting down ...", &Font16, BLACK, WHITE);
    	LCD_2IN_Display((UBYTE *)BlackImage);
	DEV_Delay_ms(3000);

	Paint_Clear(BLACK);
	Paint_DrawRectangle(10, 10, 310, 230, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(15, 15, 305, 225, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(25, 40, "Now shutting down ...", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 70, "All settings and messages", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 90, "have been saved.", &Font16, BLACK, WHITE);
	LCD_2IN_Display((UBYTE *)BlackImage);
	DEV_Delay_ms(3000);

	Paint_Clear(BLACK);
	Paint_DrawRectangle(10, 10, 310, 230, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(15, 15, 305, 225, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(25, 40, "Now shutting down ...", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 70, "All settings and messages", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 90, "have been saved.", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 120, "Please flip switch to", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 140, "power off device.", &Font16, BLACK, WHITE);
	LCD_2IN_Display((UBYTE *)BlackImage);
	DEV_Delay_ms(3000);

	Paint_Clear(BLACK);
	Paint_DrawRectangle(10, 10, 310, 230, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(15, 15, 305, 225, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(25, 40, "Now shutting down ...", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 70, "All settings and messages", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 90, "have been saved.", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 120, "Please flip switch to", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 140, "power off device.", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 160, "and remember ...", &Font16, BLACK, WHITE);
	LCD_2IN_Display((UBYTE *)BlackImage);
	DEV_Delay_ms(3000);

	Paint_Clear(BLACK);
	Paint_DrawRectangle(10, 10, 310, 230, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(15, 15, 305, 225, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(25, 40, "Now shutting down ...", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 70, "All settings and messages", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 90, "have been saved.", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 120, "Please flip switch to", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 140, "power off device.", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(25, 160, "and remember ...", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(70, 180, "F.L.A.M.I.N.G.O.s", &Font16, BLACK, WHITE);
	Paint_DrawString_EN(80, 200, "never say die!", &Font16, BLACK, WHITE);
    	LCD_2IN_Display((UBYTE *)BlackImage);

    	free(BlackImage);
    	BlackImage = NULL;
	DEV_ModuleExit();
}

#endif
