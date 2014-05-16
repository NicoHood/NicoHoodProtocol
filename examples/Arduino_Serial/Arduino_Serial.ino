/*
 Arduino_Serial.ino - SerialProtocol library - demo
 Copyright (c) 2014 NicoHood.  All right reserved.
 Daniel Garcia from the FASTLED library helped me with this code
 
 ================================================================================
 Instructions
 ================================================================================
 
 *Find Serial device on Raspberry with ~ls /dev/tty*
 ARDUINO_UNO "/dev/ttyACM0"
 FTDI_PROGRAMMER "/dev/ttyUSB0"
 HARDWARE_UART "/dev/ttyAMA0"
 
 How to setup:
 Install Serial Protocol with
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/
 sudo make install
 
 Compile you program of choice:
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/
 sudo make serialtest
 sudo make ambilight
 
 Start your program of choice (close with crtl+c):
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/
 sudo examples/Arduino_Serial/Pi_Serial.o
 sudo examples/Ambilight_Serial/Ambilight_Serial.o
 
 General information about compiling:
 If the Arduino and Raspberry program are placed in the same path make sure to name them different
 You can add other entrys to the makefile with your own sketches, or just create a new makefile
 You can also compile every sketch with the following compiler flags (install wiringPi and SerialProtocol first):
 gcc -o outpath/outname.o inpath/inname.cpp -DRaspberryPi -lwiringPi -lSerialProtocol -pedantic -Wall
 For uninstall just delete the files that the makefile writes. Havent created a uninstall yet, sorry.
 You might want to checkout the makefile in the Arduino_Serial example
 
 Direct compile and start program:
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/examples/Arduino_Serial
 sudo gcc -o Pi_Serial.o Pi_Serial.cpp -pedantic -Wall -lwiringPi -lSerialProtocol -DRaspberryPi  && sudo ./Ambilight_Serial.o
 sudo ./Pi_Serial
 or optional with the makefile for this example:
 sudo make && sudo ./Pi_Serial
 
 */

//Hardware Setup
int buttonPin=8;
unsigned long lastButton=0; //debounce
int ledPin = 9;
int sensorPin=18;
long previousMillis = 0; 
long previousMillis2 = 0; 

//Protocol
#include "SerialProtocol.h"


void setup()
{
  // button setup with pullup
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

  delay(1000);
  while(!Serial && !Serial1); //for Leonardo wait for Pi or PC

  //still need to setup the Serial for this
  Serial1.begin(9600); //Protocol communication
  Serial.begin(9600);  //debug output

  Serial.println("Arduino Startup!");

  // Pass the Serial we want to communicate
  // you can also call the function in you loop
  // to change the Serial at any time again
  // this will cancel any pending data reads.
  Protocol.setSerial(Serial1);

}


void loop()
{
  if(digitalRead(buttonPin)==LOW && millis()-lastButton>=200){
    Serial.println("Button");

    // Write some testvalues
    // Syntax: Address 1-64, 32bit Data (unsigned long)
    Protocol.sendAddress(1, random(1000)); 

    // Write a direct 4 bit command
    // Syntax: Command: 1-16
    Protocol.sendCommand(2);

    // for debounce
    lastButton=millis(); 
  }

  // send analogread every 3 sec
  if(millis() - previousMillis > 3000) {
    Protocol.sendAddress(2, analogRead(sensorPin)); 
    previousMillis = millis();   
  }

  // send Ping every 10 sec
  if(millis() - previousMillis2 > 10000) {
    Protocol.sendCommand(1);
    previousMillis2 = millis();   
  }

  // read Serial
  // if you dont need an error check you can build an if() or while()
  // around the read function to save time. while for reading every incoming data
  // pay attention if you use while if you get corrupted data with some correct
  // then the while might never end until the stream does.
  Protocol.read();

  // check for Errors (explained in dokumentation/.h)
  uint8_t error = Protocol.getErrorLevel();
  if(error){
    // print how many errors occured and the special types
    Serial.println("Errors: ");
    Serial.println(error&PROTOCOL_ERR_MAX);
    if((error&PROTOCOL_ERR_LEAD)==PROTOCOL_ERR_LEAD) Serial.println("Overwriting lead");
    if((error&PROTOCOL_ERR_DATA)==PROTOCOL_ERR_DATA) Serial.println("Data Err");
    if((error&PROTOCOL_ERR_END)==PROTOCOL_ERR_END) Serial.println("End Err");
  }

  switch(Protocol.getAddress()){
  case 0:
    // No Address
    break;
  case 1:
    analogWrite(ledPin, Protocol.getData());
    break;
  default:
    //not used
    break;
  }//end switch

  switch(Protocol.getCommand()){
  case 0:
    // No Command
    break;
  case 1:
    Serial.println("Ping!");
    break;
  default:
    //not used
    break;
  }//end switch

}

