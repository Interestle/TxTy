#ifndef LCD_H
#define LCD_H
/******************************************************************************
  This library is an amalgamation of a few different libraries I found to 
  interface with our lcd displays. I wanted it to be designed in such a way
  that a limited number of header files was included, and I wasn't impressed 
  the source code that was supplied with the displays. 
  
  Sources:
	Adafruit's ST7735 Library: https://github.com/adafruit/Adafruit-ST7735-Library
	Adafruit's GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
	Waveshare 2" LCD wiki: https://www.waveshare.com/wiki/2inch_LCD_Module
******************************************************************************/

/* Dependencies */
/* TODO: Need to figure out how to use properly */
#include <wiringPi.h> 
#include <wiringPiSPI.h> 

/* Physical dimensions of the LCD module */

#define LCD_WIDTH  240
#define LCD_HEIGHT 320

/* Pin definitions for the connectors. Uses the same ones from waveshare *
 * Note that these are not the physical pins, but the GPIO pins          */

/*              GPIO -  Physical */
#define LCD_CS   8    /* 24 */
#define LCD_DC  25    /* 22 */
#define LCD_RST 27    /* 13 */
#define LCD BL  18    /* 12 */

/* Various Commands that the ST7789 Understands */

#define ST7789_NOP       0x00 /* No Operation */
#define ST7789_SWRESET   0x01 /* Software Reset */
#define ST7789_RDDID     0x04 /* Read Display ID */
#define ST7789_RDDST     0x09 /* Read Display Status */
#define ST7789_RDDPM     0x0A /* Read Diplay Power Mode */
#define ST7789_RDDMADCTL 0x0B /* Read Display MADCTL */
#define ST7789_RDDCOLMOD 0x0C /* Read Display Pixel Format */
#define ST7789_RDDIM     0x0D /* Read Display Image Mode */
#define ST7789_RDDSM     0x0E /* Read Display Signal Mode */
#define ST7789_RDDSDR    0x0F /* Read Display Self-Diagnostic Result */
#define ST7789_SLPIN     0x10 /* Sleep In */
#define ST7789_SLPOUT    0x11 /* Sleep Out */
#define ST7789_PTLON     0x12 /* Partial Display Mode On */
#define ST7789_NORON     0x13 /* Normal Display Mode On */
#define ST7789_INVOFF    0x20 /* Display Inversion Off */
#define ST7789_INVON     0x21 /* Display Inversion On */
#define ST7789_GAMSET    0x26 /* Gamma Set */
#define ST7789_DISPOFF   0x28 /* Display Off */
#define ST7789_DISPON    0x29 /* Display On */
#define ST7789_CASET     0x2A /* Column Address Set */
#define ST7789_RASET     0x2B /* Row Address Set */
#define ST7789_RAMWR     0x2C /* Memory Write */
#define ST7789_RAMRD     0x2E /* Memory Read */
#define ST7789_PRLAR     0x30 /* Partial Area */
#define ST7789_VSCRDEF   0x33 /* Vertical Scrolling Definition */
#define ST7789_TEOFF     0x34 /* Tearing Effect Line Off */
#define ST7789_TEON      0x35 /* Tearing Effect Line On */
#define ST7789_MADCTL    0x36 /* Memory Data Access Control */
#define ST7789_VSCSAD    0x37 /* Vertical Scroll Start Address of RAM */
#define ST7789_IDMOFF    0x38 /* Idle Mode Off */
#define ST7789_IDMON     0x39 /* Idle Mode On */
#define ST7789_COLMOD    0x3A /* Interface Pixel Format */
#define ST7789_WRMEMC    0x3C /* Write Memory Continue */
#define ST7789_RDMEMC    0x3E /* Read Memory Continue */
#define ST7789_STE       0x44 /* Set Tear Scanline */
#define ST7789_GSCAN     0x45 /* Get Scanline */
#define ST7789_WRDISBV   0x51 /* Write Display Brightness */
#define ST7789_RDDISBV   0x52 /* Read Display Brightness Value */
#define ST7789_WRCTRLD   0x53 /* Write CTRL Display */
#define ST7789_RDCTRLD   0x54 /* Read CTRL Value Display */
#define ST7789_WRCACE    0x55 /* Write Content Adaptive Brightness Control and Color Enhancement */
#define ST7789_RDCABC    0x56 /* Read Content Adaptive Brightness Control */
#define ST7789_WRCABCMB  0x5E /* Write CABC Minimum Brightness */
#define ST7789_RDCABCMB  0x5F /* Read CABC Minimum Brightness */
#define ST7789_RDABCSDR  0x68 /* Read Automatic Brightness Control Self-Diagnostic Result */
#define ST7789_PORCTRL   0xB2 /* Porch Setting */
#define ST7789_GCTRL     0xB7 /* Gate Control */
#define ST7789_VCOMS     0xBB /* VCOM Setting */
#define ST7789_LCMCTRL   0xC0 /* LCM Control */
#define ST7789_VDVVRHEN  0xC2 /* VDV and VRH Command Enable */
#define ST7789_VRHS      0xC3 /* VRH Set */
#define ST7789_VDVS      0xC4 /* VDV Set */
#define ST7789_RDID1     0xDA /* Read ID1 */
#define ST7789_RDID2     0xDB /* Read ID2 */
#define ST7789_RDID3     0xDC /* Read ID3 */
#define ST7789_RDID4     0xDD /* Read ID4 */


/*
 * LCD Register initialization. 
 */
void LCD_init(void)
{
  LCD_reset();
  
  /* Let's start with how Waveshare does things and go from there. */
  LCD_write_command(ST7789_COLMOD);
  LCD_write_command(0x05); /* Adafruit does it as 0x55 */

  LCD_write_command(ST7789_MADCTL);
  LCD_write_data(0x00); /* Adafruit initially sends it 0x08 */

  LCD_write_command(ST7789_CASET);
  LCD_write_data(0x00);
  LCD_write_data(0x00);
  LCD_write_data(0x01); /* Adafruit uses 0 */
  LCD_write_data(0x3F); /* 63, Adafruit uses 240 ??? */

  LCD_write_command(ST7789_RASET);
  LCD_write_data(0x00);
  LCD_write_data(0x00);
  LCD_write_data(0x00); /* Adafruit: 320 >>8 */
  LCD_write_data(0xEF); /* 239, Adafruit: 320 & 0xFF (0x40) ??? */

  /* Porch Setting, the manual has it this sequence at power on, Adafruit * 
   * doesn't mess with it, but waveshare does. Need to check later        */
  LCD_write_command(ST7789_PORCTRL);
  LCD_write_data(0x0C);
  LCD_write_data(0x0C);
  LCD_write_data(0x00);
  LCD_write_data(0x33);
  LCD_write_data(0x33);

  /* waveshare also sets this to its initial value of 0x35                 *
   * VGH should be 13.26 V, VGL should be -10.43 V ... Whatever that means */
  LCD_write_command(ST7789_GCTRL);
  LCD_write_data(0x35);

  /* Again, something waveshare does. They set VCOM to 0.85 V.                *
   * I don't know why, I don't know what it does. Regardless, here we are.    */
  LCD_write_command(ST7789_VCOMS);
  LCD_write_data(0x1F);
 
  /* Default power settings again */
  LCD_write_command(ST7789_LCMCTRL);
  LCD_write_data(0x2C);

  /* Now this command makes no sense and was also copied from waveshare.           *
   * What makes it weird is that it needs 2 commands, but waveshare only does one. *
   * Because of that, I'm actually just going to comment this one out and find     *
   * out if it's really needed.                                                    *
   * LCD_write_command(ST7789_VDVVRHEN);                                           *
   * LCD_write_data(0x01);                                                         */

  /* One of the few things waveshare deviates from. Why? */
  LCD_write_command(ST7789_VRHS);
  LCD_write_data(0x12); /* Default is 0x0B */

  LCD_write_command(ST7789_VDVS);
  LCD_write_data(0x20); /* Default value */

  /* I'm going to go through waveshare's stuff for the defaults, and just *
   * list them instead of writing them.                                   *
   * FRCTRL2 - Frame Rate Control. Defaults to 60 Hz, waveshare set to 60 *
   * PWECTRL1 - Power Control 1, defaults to 0xA4, 0xA1. Normal           *
   * PVGAMCTRL - Positive Voltage Gamma Control. Requires PHD to understand *
   * NVGAMCTRL - Negative Voltage Gamma Control. See above                */

  LCD_write_command(ST7789_INVON); /* Adafruit comments just hack on this, so it's needed? */
  LCD_write_command(ST7789_SLPOUT);
  LCD_write_command(ST7789_DISPON);
}

/*
 * Hardware Reset
 */
static void LCD_reset(void)
{
  digitalWrite(LCD_CS, 0);
  delay(100);
  digitalWrite(LCD_RST, 1);
  delay(100);
  digitalWrite(LCD_RST, 0);
  delay(100);
}

/*
 * Write a command to the LCD Panel
 */
static void LCD_write_command(uint8_t data)
{
  digitalWrite(LCD_CS, 0);
  digitalWrite(LCD_DC, 0);
  wiringPiSPIDataRW(0, &data, 1);
}

/*
 * Write a byte of data to the LCD
 */
static void LCD_write_data(uint8_t data)
{
  digitalWrite(LCD_CS, 0);
  digitalWrite(LCD_DC, 1);
  wiringPiSPIDataRW(0, &data, 1);
  digitalWrite(LCD_CS, 1);
}


#endif /* !LCD_H */
