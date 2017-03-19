#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

// for Feather 32u4 Radio
#define RFM69_CS      8
#define RFM69_IRQ     7
#define RFM69_IRQN    4  // Pin 7 is IRQ 4!
#define RFM69_RST     4

#define SD_CS         10
#define VBATPIN       A9

Adafruit_BNO055 bno = Adafruit_BNO055();

RTC_PCF8523 rtc;

String dataString;

bool sd_error = false;
bool wireless_error = false;
bool rtc_error = false;
bool battery_error = false;
bool accel_error = false;

uint8_t led_r = 5;
uint8_t led_g = 9;
uint8_t led_b = 6;

unsigned long last_check = 0;

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

void writeToSD(String data) {
  if (!sd_error) {
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      sd_error = false;
      dataFile.println(data);
      dataFile.close();
    } else {
      sd_error = true;
      Serial.println("#SD-ERROR: problem opening file");
    }
  }
}

void measureBatteryVoltage() { 
    float measuredvbat = 0;
    measuredvbat = analogRead(VBATPIN);
//    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage
    if (measuredvbat < 3 && measuredvbat > 1) {
      battery_error = true;
      Serial.print("#VBat: " ); Serial.println(measuredvbat);
    } else {
      battery_error = false;
    }
}

void displayStatus() {

  if (sd_error | wireless_error | rtc_error | battery_error) {
    // bad thing, steady red
    digitalWrite(led_r, LOW);
    digitalWrite(led_g, HIGH);
    digitalWrite(led_b, HIGH);
  } else {
    //nominal, steady green
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, HIGH);
  }
  
}

void errorCheckandRecover() {
  if (millis() - last_check > 5000) {
    last_check = millis();
    measureBatteryVoltage();
    if (sd_error) {
      Serial.println("#SD-ERROR: found error, looking to recover");
      if (SD.begin(SD_CS)) {
        Serial.println("#SD-ERROR: resolved");
        sd_error = false;
      }
    }
    if (rtc_error) {
      Serial.println("#RTC-ERROR: found error, looking to recover");
      if (rtc.begin()) {
        Serial.println("#RTC-ERROR: resolved");
        rtc_error = false;
      }
    }
    if (wireless_error) {
      Serial.println("#RADIO-ERROR: found error, looking to recover");
      
      // Hard Reset the RFM module
      pinMode(RFM69_RST, OUTPUT);
      digitalWrite(RFM69_RST, HIGH);
      delay(100);
      digitalWrite(RFM69_RST, LOW);
      delay(100);
    
      // Initialize radio
      radio.initialize(FREQUENCY,NODEID,NETWORKID);
      if (IS_RFM69HCW) {
        radio.setHighPower();    // Only for RFM69HCW & HW!
      }
      radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
      
      radio.encrypt(ENCRYPTKEY);
    }
    if (battery_error) {
      Serial.println("#BATTERY LOW!");
    }
    if (accel_error) {
      Serial.println("#ACCEL-ERROR: found error, looking to recover");
      if(bno.begin()) {
        Serial.println("#ACCEL-ERROR: resolved");
        accel_error = false;
      }
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("Feather RFM69HCW Transmitter");

  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_b, OUTPUT);
  
  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
  
  radio.encrypt(ENCRYPTKEY);
  
  Serial.print("\nTransmitting at ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");

  // initialize SD card
  if (!SD.begin(SD_CS)) {
    sd_error = true;
  }

  //begin RTC
  if (!rtc.begin()) {
    rtc_error = true;
  }

//  if (!rtc.initialized()) {
//    Serial.println("RTC is NOT running!");
//    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  }

  /* Initialise the accelerometer */
  if(!bno.begin())
  {
    accel_error = true;
  }
}

void loop() {
  
  errorCheckandRecover();
  displayStatus();
  
  // get time
  DateTime now = rtc.now();
  //format a data string
  dataString = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
  
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  
  imu::Vector<3> linearAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  dataString += " #xyz: " + String(linearAccel.x()) + "," + String(linearAccel.y()) + "," + String(linearAccel.z()) + " #ypr: " + String(euler.x()) + "," + String(euler.y()) + "," + String(euler.z());

  //write to SD
  writeToSD(dataString);
  Serial.println(dataString);
    
  char radiopacket[60];
  dataString.toCharArray(radiopacket,60);//"Hello World #";
  
  if (radio.sendWithRetry(RECEIVER, radiopacket, strlen(radiopacket))) { //target node Id, message as string or byte array, message length
    Serial.println("OK"); // why isn't it printing this?
  }
//  radio.send(RECEIVER, radiopacket, strlen(radiopacket));

  radio.receiveDone(); //put radio in RX mode
  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU

  delay(20);  // Wait 1 second between transmits, could also 'sleep' here!
}
