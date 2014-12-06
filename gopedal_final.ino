
/* 
* Author: Zohaib Ahmed
*/

#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include "Adafruit_FONA.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <EasyTransfer.h>

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// for the TFT
#define TFT_DC 9
#define TFT_CS 10

// this is a large buffer for replies
char replybuffer[255];
char * r2;
char location[30];
char latdegs[10];
char longdegs[10];

unsigned long timer = millis();

EasyTransfer ET;

struct RECEIVE_DATA_STRUCTURE{
  float averagespeed;
  float currentspeed;
  float totaldistance;
  int proximity1; 
  int proximity2; 
};

RECEIVE_DATA_STRUCTURE mydata;

bool locked = false;

float old_latitude = 0.0;
float old_longitude = 0.0;

uint8_t last_sms = 0; 
float old_average = -1;
float old_current = -1;
float old_distance = -1;
uint8_t n;

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(&fonaSS, FONA_RST);

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

// If using software serial, keep this line enabled
// (you can change the pin numbers to match your wiring):
SoftwareSerial mySerial(7, 6);

// If using hardware serial (e.g. Arduino Mega), comment out the
// above SoftwareSerial line, and enable this line instead
// (you can change the Serial number to match your wiring):

//HardwareSerial mySerial = Serial1;

Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences. 
#define GPSECHO  true

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;
void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy

void setup()  
{
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  ET.begin(details(mydata), &Serial);

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  // the nice thing about this code is you can have a timer0 interrupt go off
  // every 1 millisecond, and read data from the GPS for you. that makes the
  // loop code a heck of a lot easier!
  useInterrupt(true);

  
  // See if the FONA is responding
  fona.begin(4800);  // make it slow so its easy to read!
  n = fona.getNetworkStatus();

  // Set up the TFT
  tft.begin();

  // read diagnostics (optional but can help debug problems)

  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(10, 0);
 
  tft.println("DISTANCE");

  // print the total distance
  tft.setCursor(110, 0);

  tft.println("CURRENT SPEED");

  // print the current speed
  tft.setCursor(210, 0);
  
  tft.println("AVERAGE SPEED");

  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
}


// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
#ifdef UDR0
  if (GPSECHO)
    if (c) UDR0 = c;  
    // writing direct to UDR0 is much much faster than Serial.print 
    // but only one character can be written at a time. 
#endif
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

void loop()                     // run over and over again
{
  // Listen to the master... be one with the master.
  if(ET.receiveData()){
    // print the current speed
    if (mydata.averagespeed != old_average) {
      tft.fillRect(10, 10, 100, 30, ILI9341_BLACK);

      tft.setCursor(10, 10);
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_WHITE);
      tft.println(mydata.averagespeed);
      old_average = mydata.averagespeed;
    }
    
    if (mydata.currentspeed != old_current) {
      tft.fillRect(110, 10, 100, 30, ILI9341_BLACK);
      tft.setTextSize(3);
      tft.setTextColor(ILI9341_BLUE);

      // print the total distance
      tft.setCursor(tft.width()/2 - 50, 10);
      tft.println(mydata.currentspeed);
      old_current = mydata.currentspeed;
    }

    if (mydata.totaldistance != old_distance) {
      tft.fillRect(210, 10, 100, 30, ILI9341_BLACK);
      // print the current speed
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(tft.width() - 80, 10);
      tft.println(mydata.totaldistance);
      old_distance = mydata.totaldistance;
    }

    if (mydata.proximity1 == 1) {
      // draw left rect
      tft.fillRect(0, tft.height() - 40, tft.width()/2, 40, ILI9341_RED);

    } else {
      // nothing is there...
      tft.fillRect(0, tft.height() - 40, tft.width()/2, 40, ILI9341_BLACK);
    }

    if (mydata.proximity2 == 1) {
      // draw right rect
      tft.fillRect(tft.width()/2, tft.height() - 40, tft.width()/2, 40, ILI9341_RED);

    } else {
      // nothing is there...
      tft.fillRect(tft.width()/2, tft.height() - 40, tft.width()/2, 40, ILI9341_BLACK);

    }

  }

  // in case you are not using the interrupt above, you'll
  // need to 'hand query' the GPS, not suggested :(
  // if (! usingInterrupt) {
  //   // read data from the GPS in the 'main loop'
  //   char c = GPS.read();
  // }
  
  // if a sentence is received, we can check the checksum, parse it...
  // if (GPS.newNMEAreceived()) {
  //   // a tricky thing here is if we print the NMEA sentence, or data
  //   // we end up not listening and catching other sentences! 
  //   // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
  //   //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
  //   if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
  //     return;  // we can fail to parse a sentence in which case we should just wait for another
  // }
  Serial.println(latdegs);
  Serial.println(longdegs);

  if (locked == true && (old_latitude != GPS.latitudeDegrees || old_longitude != GPS.longitudeDegrees) && (n == 1)) {
    // notify the user that their bike has moved...
    fona.sendSMS("6472985142", "Your bike is on the move! Please check the GoPedal app.");
  }
  
  if (millis() - timer > 3000 && (n == 1)) {
    uint8_t smsn = fona.getNumSMS();

    if (last_sms != smsn) {
      // Retrieve SMS value.
      // read the last recieved SMS
      uint16_t smslen;
      fona.readSMS(smsn, replybuffer, 250, &smslen);
     
      // chopN(replybuffer, 38);
      r2 = replybuffer + 38;

      if (strcmp(r2, "LOCATION") == 0) {
       // send current location back to server
       //Serial.println(F("Sending Location To Server..."));
       snprintf(location, sizeof(location), "LOCATION %d, %d", latdegs, longdegs);

       fona.sendSMS("4378002797", location);

       memset(location, 0, sizeof location);
      } else if (strcmp(r2, "LOCK") == 0) {
        if (locked == true) {
          locked = false;
        } else {
          locked = true;
          old_latitude = GPS.latitudeDegrees;
          old_longitude = GPS.longitudeDegrees;
        }
      } else {
        tft.fillRect(0, tft.height()/2 - 60, tft.width(), 120, ILI9341_BLACK);

        tft.setCursor(0, tft.height()/2 - 40);
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(4);
        tft.println(r2);
      }
      
    }
    last_sms = smsn;
    timer = millis();
  }
}

