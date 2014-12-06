#include <SPI.h>
#include <ADXL362.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <NewPing.h>
#include <EasyTransfer.h>

//create object
EasyTransfer ET;

#define TRIGGER_PIN1  7  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN1     8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define TRIGGER_PIN2  5  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN2     6  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar1(TRIGGER_PIN1, ECHO_PIN1, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
NewPing sonar2(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//Hall Sensor Constants
const float pi = 3.1415926535897932384626433832795; //PI
const float r = 0.00004; //Radius of 40 centimetres (in kilometres)
const float distance = 2.0 * PI * r;

// Analog I/O
const int leftForce = 0;
const int rightForce = 1;
const int photocell = 2;

// Digital I/O
const int hallEffect = 9;

struct SEND_DATA_STRUCTURE{
 float averagespeed;
 float currentspeed;
 float totaldistance;
 int proximity1; // the way I'm thinking about this is just flags, 0 for nothing, 1 for left, 2 for right? 
 int proximity2;
} SEND_DATA_STRUCTURE_default = {0, 0, 0, 0, 0}; //Initialize the values to 0.

//give a name to the group of data
SEND_DATA_STRUCTURE mydata = SEND_DATA_STRUCTURE_default;

// Global variables
int leftForceVal, rightForceVal, leftForceValPrev, rightForceValPrev;
boolean leftSignalOn, rightSignalOn;
unsigned long leftSignalTime, rightSignalTime;
int leftSignalMod, leftSignalModPrev, rightSignalMod, rightSignalModPrev;
boolean leftSignalLightOn, rightSignalLightOn;
boolean turned;
boolean cycleCompleted = true, startMeasuring = false;
int photocellVal;
int hallEffectVal = LOW;
unsigned long totalHallEffectTime = 0, currentHallEffectTime = millis();
float totalDistance = 0, currentSpeed, avgSpeed;
ADXL362 xl;
int16_t xRaw, yRaw, zRaw, temp;
int xValue, yValue, zValue;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup() {
  pinMode(hallEffect,INPUT);
  Serial.begin(115200);
  xl.begin(10);
  xl.beginMeasure();
  pwm.begin();
  pwm.setPWMFreq(1600);
  uint8_t twbrbackup = TWBR;
  TWBR = 12;
  leftSignalOn = false;
  rightSignalOn = false;
  pwm.setPin(13, 0, false);
  pwm.setPin(14, 0, false);
  pwm.setPin(15, 0, false);
  
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(mydata), &Serial);  
}

void loop() {
  
//  Serial.println(mydata.currentspeed);
//  Serial.println(mydata.averagespeed);
//  Serial.println(mydata.totaldistance);
//  Serial.println(mydata.proximity1);
//  Serial.println(mydata.proximity2);
  
  unsigned int uS1 = sonar1.ping(); // Send ping, get ping time in microseconds (uS).
  unsigned int uS2 = sonar2.ping(); // Send ping, get ping time in microseconds (uS).
  unsigned int cm1 = uS1 / US_ROUNDTRIP_CM; //Value of prox1 data in cm.
  unsigned int cm2 = uS2 / US_ROUNDTRIP_CM; //Value of prox2 data in cm.
  
  if(cm1 > 0 && cm1 <= 75){ mydata.proximity1 = 1; }
  else { mydata.proximity1 = 0; }
  if(cm2 > 0 && cm2 <= 75){ mydata.proximity2 = 1; }
  else { mydata.proximity2 = 0; }
  
  hallEffectVal = digitalRead(hallEffect);
  
  if(hallEffectVal == HIGH)
  {    
    if(!startMeasuring){
      startMeasuring = true;
    }
    else if(startMeasuring && !cycleCompleted){
      //Serial.println("North");
      
      cycleCompleted = true;
      
      currentHallEffectTime = millis() - currentHallEffectTime;
      totalHallEffectTime += currentHallEffectTime;
      
      totalDistance += distance;
      
      currentSpeed = (distance / currentHallEffectTime)*1000*3600;
      avgSpeed = (totalDistance / totalHallEffectTime)*1000*3600;
      
//      Serial.print("Distance: ");
//      Serial.println((totalDistance * 1000), 8); //In metres
//      
//      Serial.print("Current Speed: ");
//      Serial.println(currentSpeed, 8); //In km
//      
//      Serial.print("Average Speed: ");
//      Serial.println(avgSpeed, 8); //In km/h
      
      mydata.averagespeed = (totalDistance * 1000);
      mydata.currentspeed = currentSpeed;
      mydata.totaldistance = avgSpeed;
    }

  }
  else if(hallEffectVal == LOW)
  {    
      if(cycleCompleted){
        //Serial.println("South");
        
        cycleCompleted = false;
        currentHallEffectTime = millis();
      }
      
  }
  
  ET.sendData();
  
  photocellVal = analogRead(photocell);
  if (photocellVal > 700) {
    pwm.setPin(13, 4095, false);
  } else {
    pwm.setPin(13, 0, false);
  }
  
  leftForceValPrev = leftForceVal;
  leftForceVal = analogRead(leftForce);
  if ((leftForceVal > 500) && (leftForceValPrev < 500)) {
    if (leftSignalOn) {
      leftSignalOn = false;
      leftSignalLightOn = false;
      pwm.setPin(14, 0, false);
    } else {
      leftSignalOn = true;
      leftSignalTime = millis();
      leftSignalLightOn = true;
      leftSignalMod = 0;
      leftSignalModPrev = 0;
      pwm.setPin(14, 4095, false);
      turned = false;
    }
  }
  if (leftSignalOn) {
    leftSignalModPrev = leftSignalMod;
    leftSignalMod = (millis() - leftSignalTime) % 250;
    if (leftSignalModPrev > leftSignalMod) {
      leftSignalLightOn = !leftSignalLightOn;
      if (leftSignalLightOn) {
        pwm.setPin(14, 4095, false);
      } else {
        pwm.setPin(14, 0, false);
      }
    }
    xl.readXYZTData(xRaw, yRaw, zRaw, temp);
    xValue = map(xRaw, -4000, 4000, -20, 20);
    yValue = map(yRaw, -4000, 4000, -20, 20);
    zValue = map(zRaw, -7000, 7000, -20, 20);
    Serial.print("X:");
    Serial.print(xValue);
    Serial.print(", Y:");
    Serial.print(yValue);
    Serial.print(", Z:");
    Serial.println(zValue);
    if ((zValue >= 2) || (zValue <= -2)) {
      turned = true;
    } else if ((zValue <= 1) && (zValue >= -1) && (turned)) {
      leftSignalOn = false;
      leftSignalLightOn = false;
      pwm.setPin(14, 0, false);
    }
  }
  
  rightForceValPrev = rightForceVal;
  rightForceVal = analogRead(rightForce);
  if ((rightForceVal > 500) && (rightForceValPrev < 500)) {
    if (rightSignalOn) {
      rightSignalOn = false;
      rightSignalLightOn = false;
      pwm.setPin(15, 0, false);
    } else {
      rightSignalOn = true;
      rightSignalTime = millis();
      rightSignalLightOn = true;
      rightSignalMod = 0;
      rightSignalModPrev = 0;
      pwm.setPin(15, 4095, false);
      turned = false;
    }
  }
  
  if (rightSignalOn) {
    rightSignalModPrev = rightSignalMod;
    rightSignalMod = (millis() - rightSignalTime) % 250;
    if (rightSignalModPrev > rightSignalMod) {
      rightSignalLightOn = !rightSignalLightOn;
      if (rightSignalLightOn) {
        pwm.setPin(15, 4095, false);
      } else {
        pwm.setPin(15, 0, false);
      }
    }
    xl.readXYZTData(xRaw, yRaw, zRaw, temp);
    xValue = map(xRaw, -4000, 4000, -20, 20);
    yValue = map(yRaw, -4000, 4000, -20, 20);
    zValue = map(zRaw, -7000, 7000, -20, 20);
    if ((zValue >= 2) || (zValue <= -2)) {
      turned = true;
    } else if ((zValue <= 1) && (zValue >= -1) && (turned)) {
      rightSignalOn = false;
      rightSignalLightOn = false;
      pwm.setPin(15, 0, false);
    }
  }
  
  delay(25); //The proximity sensors need this.
}
