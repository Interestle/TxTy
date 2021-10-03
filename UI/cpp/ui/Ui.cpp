/*****************************************************************************
* | File      	:   Ui.h
* | Author      :   Seth Jackson   
* | Info       	: Basic UI using waveshare's example code for the their 2in
*		  LCD module. It uses 8 fuctions starting with LCD_INIT and 
*		  LCD_exit both need to be in the LCD_INIT sets up everything
*		  needed to start painting but malloc's a pointer that needs
*		  to be freed in LCD_exit. LCD_set_font takes an int representing
*		  the font size and will change the text size displayed on the UI
*		  unless it is not a size I have built in. If not then it will 
*		  be set to 16. LCD_battery takes an int representing the 
*		  percentage of charge left. If the percentage is 20 or less
*		  it will change the color of the battery icon to red. The LCD_up
*		  fuction will move the text up one and the LCD_down moves the text
*		  down one. LCD_refresh is mostly just used for the other fuctions
*		  to display the changes.
*
void LCD_INIT(void);
void LCD_text(std::vector<std::string> &texts);
void LCD_set_font(int size); 
void LCD_battery(int charge);
void LCD_up(void);
void LCD_down(void);
void LCD_refresh(void);
void LCD_exit(void);

*----------------
* |	This version:   V1.0
* | Date        :   2021-09-26
* | Info        :   Basic version
*
******************************************************************************/

#include "Ui.h"
#include <stdio.h>	//printf()
#include <stdlib.h>	//exit()
#include <signal.h>     //signal()
#include <math.h>

UWORD *BlackImage;
std::vector<std::string> text;
int battery = 305;
UWORD battery_color = GREEN;
sFONT font = Font16;	
unsigned int text_limit = 24;
size_t start = 0;
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
	LCD_2IN_Clear(WHITE);
	LCD_SetBacklight(1010);
	
    	UDOUBLE Imagesize = LCD_2IN_HEIGHT*LCD_2IN_WIDTH*2;
    	if((BlackImage = (UWORD *)malloc(Imagesize)) == NULL) 
	{
        	printf("Failed to apply for black memory...\r\n");
        	exit(0);
    	}
	
    	// /*1.Create a new image cache named IMAGE_RGB and fill it with white*/
    	Paint_NewImage(BlackImage, LCD_2IN_WIDTH, LCD_2IN_HEIGHT, 90, WHITE, 16);
    	Paint_Clear(WHITE);
	Paint_SetRotate(ROTATE_270);
    	
	// /* GUI */
    	printf("drawing...\r\n");
    	
	// /*2.Drawing on the image*/

	LCD_refresh();	
    
}

/*
 * Set font of text.
 * text_limit is used to wrap the text around before it reaches the scroll bar.
 * each text_limit was counted by printing to screen and counting every character then fit on the screen.
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

	LCD_refresh();
	

}

/*
 * Sets text printed to the UI.
 * Takes a vector of strings then clears the current text before
 * inserting the strings from the vector.
 */
void LCD_text(std::vector<std::string> &texts)
{
	text.clear();
	text.insert(text.end(), texts.begin(), texts.end());
	LCD_refresh();
//	printf("page length %f\n", page_size);
//	printf("number of rows %f\n", row_num);
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
		printf("Not a valid color.\r\n");
		free(BlackImage);
    		BlackImage = NULL;
		exit(1);
	}

	LCD_refresh();

}

/*
 * Changes UI to move up in the text.
 * if already at the first text it won't do anything.
 */
void LCD_up(void)
{
	if (0 >= start)
	{
		start = 0;
	}
	else if (start < text.size())
	{
		start--;
	}
	else
	{
		start = text.size() - 1;
	}
	
	LCD_refresh();

}

/*
 * Changes UI to move down in the text.
 * if already at the last text it won't do anything.
 */
void LCD_down(void)
{
	if (0 > start)
	{
		start = 0;
	}
	else if (start < (text.size() - 1))
	{
		start++;
	}
	else
	{
		start = text.size() - 1;
	}
	
	LCD_refresh();

}

/*
 * Refresh screen.
 */
void LCD_refresh(void)
{
	Paint_Clear(WHITE); // clear screen and paint it white to make a nice canvas.

	int row = 35; // used to messure where to print text to screen.
	double start_row; // used to find first line that is actually printed to the screen.
	int row_if_no_start = 35; /* this is also counting to where to print to the screen but starting from zero
				  this is use to mesure how many rows fits on the screen.*/
	row_num = 0; // used to count all rows. This is needed for page label math.
	page_size = 0; // used to store the amount of rows that can fit on the screen.

	for (size_t i = 0; i<text.size(); i++)
	{
		std::string temp = text[i];
		char* next_text;		

		/* when i is equal to the start index text can start being printed and start_row is equal to
		the number of rows that have been counted.*/
		if (i == start)
		{
			start_row = row_num;
		}
	
		// if the text is larger then the text_limit of the font then it needs to be split into substings smaller then the text_limit.
		while (temp.length() > text_limit)
		{
			std::size_t space = temp.find_last_of(" ", text_limit); // find the first space before the text_limit.
			std::string temp2;
	
			if ((space == std::string::npos) || (space == 0)) // if there is no space or only one at the beginning.
			{
				space = text_limit - 1;
				temp2 = temp.substr(0,space);

				/* if the next row will go past the edge of the screen then I want to remove the last four characters in the substring
				 and replace it with three dots*/
				if((row + font.Height + 10) > (240 - font.Height))
				{
					temp2.erase(temp2.length()-5,temp2.length()-1);
					temp2.append("...");
				}

				temp.erase(0,space);

			}
			else // if there is a space between words.
			{
				
				temp2 = temp.substr(0,space);

				/* if the next row will go past the edge of the screen then I want to remove the last word in the substring
				 and replace it with three dots*/
				if((row + font.Height + 10) > (240 - font.Height))
				{
					std::size_t space2 = temp2.find_last_of(" ");
					temp2.erase(space2+1,temp2.length()-1);
					temp2.append("...");
				}

				temp.erase(0,space);
				
				// delete the space from the split.	
				if (temp.length() > 3)
				{
					temp.erase(0,1);
				}

			}
			
			
			// if the next row will print of the screen then we have counted all the rows that fit on the screen.
			if((row_if_no_start > (240 - font.Height)) && (page_size == 0))
			{
				page_size = row_num;
			}
	

			// provided it won't print off the screen and we reached the start index we print to screen.
			if ((row < (240 - font.Height)) && (i >= start))
			{
				next_text = const_cast<char*>(temp2.c_str());
				Paint_DrawString_EN(0, row, next_text, &font, WHITE, BLACK);

				row += (font.Height + 10);
			}
				
			row_if_no_start += (font.Height + 10);
			row_num++;	
		}	
		
		// if the next row will print of the screen then we have counted all the rows that fit on the screen.
		if((row_if_no_start > (240 - font.Height)) && (page_size == 0))
		{
			page_size = row_num;
		}	

		// provided it won't print off the screen and we reached the start index we print to screen.
		if ((row < (240 - font.Height)) && (i >= start))
		{
			next_text = const_cast<char*>(temp.c_str());
			Paint_DrawString_EN(0, row, next_text, &font, WHITE, BLACK);
	
			row += (font.Height + 10);
		}
		
		row_if_no_start += (font.Height + 10);
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
		current_page = std::to_string((int)(floor(start_row/page_size)) + 1); /* get current page by dividing number of rows before start divided by how many rows fit 
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
			
			scroll_bar_start = 45 + (start_row*5);
		}
		else
		{
			/* If there is to many texts then the limit could be reached.
			When this happens I divide the rest of the bar by number of rows
			times the number of rows before the start row. */

			scroll_bar_start = 45 + (int)(round((180/row_num)*start_row));
		}

		scroll_bar_end = 225 - (row_num*5) + (scroll_bar_start - 45);
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
	
	// Power label
	Paint_DrawRectangle(0, 0, 100, 30, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(10, 10, "power", &Font16, WHITE, BLACK);	
	
	// Reset label
	Paint_DrawRectangle(100, 0, 200, 30, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(110, 10, "reset", &Font16, WHITE, BLACK);	

	// Page label
	Paint_DrawRectangle(200, 0, 260, 30, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawString_EN(210, 10, page_label, &Font16, WHITE, BLACK);	

	// Battery
	Paint_DrawRectangle(260, 0, 320, 30, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(274, 9, 306, 26, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawRectangle(275, 10, battery, 26, battery_color, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawRectangle(306, 15, 308, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);

	// up and down labels
	Paint_DrawRectangle(290, 30, 320, 135, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(305, 80, 300, 90, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(305, 80, 310, 90, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawRectangle(290, 135, 320, 240, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(305, 195, 300, 185, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(305, 195, 310, 185, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

	// scroll bar
	Paint_DrawRectangle(270, 30, 290, 240, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
	Paint_DrawLine(280, 35, 277, 40, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(280, 35, 283, 40, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawRectangle(272, scroll_bar_start, 288, scroll_bar_end, GRAY, DOT_PIXEL_1X1, DRAW_FILL_FULL);
	Paint_DrawLine(280, 235, 277, 230, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
	Paint_DrawLine(280, 235, 283, 230, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

	// /*3.Refresh the picture in RAM to LCD*/
    	LCD_2IN_Display((UBYTE *)BlackImage);

}
/*
 * Fuction needed to free BlackImage pointer before exiting.
 */
void LCD_exit(void)
{
	/* Module Exit */
    	free(BlackImage);
    	BlackImage = NULL;
	DEV_ModuleExit();
}
