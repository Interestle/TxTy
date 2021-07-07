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
#define ST7789_RDID1     0xDA /* Read ID1 */
#define ST7789_RDID2     0xDB /* Read ID2 */
#define ST7789_RDID3     0xDC /* Read ID3 */
#define ST7789_RDID4     0xDD /* Read ID4 */





