/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <MeetAndroid.h>

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

MeetAndroid meetAndroid;
String string; 

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341 Test!"); 
  meetAndroid.registerFunction(stringValue, 'A');  
 
  tft.begin();

  // read diagnostics (optional but can help debug problems)
  // 240x320 pixels
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.drawRoundRect(0, 0, tft.width(), 40, 0, 0x7BEF);
  tft.fillRoundRect(0, 0, tft.width(), 40, 0, 0x7BEF);

  // set the top left
  tft.setCursor(5, 5);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("Current Speed");
  tft.setCursor(5, 15);
  tft.setTextSize(2);
  tft.println("30km/h");

  // set the top right
  tft.setCursor(200, 5);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("Current Distance");
  tft.setCursor(200, 15);
  tft.setTextSize(2);
  tft.println("40km");

  // set in center
  int x0 = 40, 
      y0 = tft.height()/2;

  tft.setCursor(x0, y0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("TURN RIGHT IN 50km ON");
  tft.setCursor(x0, y0 + 20);
  tft.setTextSize(3);
  tft.println("MISSISSAUGA RD");

  tft.setCursor(x0, y0 + 50);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("ETR: 33 Minutes");
}


void loop(void) {
  meetAndroid.receive();
}

/*  
 *  Will be called as soon as you receive a phone state event.
 *
 *  Use a switch statement to determine which kind of phone state event you got
 *  note: flag is in this case 'A' and numOfValues is 1 
 */
void stringValue(byte flag, byte numOfValues)
{
  // first we need to know how long the string was in order to prepare an array big enough to hold it.
  // you should know that: (length == 'length of string sent from Android' + 1)
  // due to the '\0' null char added in Arduino
  int length = meetAndroid.stringLength();
  
  // define an array with the appropriate size which will store the string
  char data[length];
  
  // tell MeetAndroid to put the string into your prepared array
  meetAndroid.getString(data);
  
  // go and do something with the string, here we simply send it back to Android
  meetAndroid.send(data);

  //Serial.println(data);
  tft.setCursor(40, tft.height() - 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println(data);
}
