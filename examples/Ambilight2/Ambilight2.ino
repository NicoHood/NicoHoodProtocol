/*
 Ambilight.ino - SerialProtocol library - Ambilight demo
 Copyright (c) 2014 NicoHood.  All right reserved.
 
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
 
 Hyperion setup (for Ambilight only)
 How to compile the new Hyperion:
 sudo apt-get update
 sudo apt-get install git cmake build-essential libprotobuf-dev libQt4-dev libusb-1.0-0-dev protobuf-compiler python-dev
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/
 # create hyperion directory and checkout the code from github
 export HYPERION_DIR="hyperion"
 sudo git clone https://github.com/tvdzwan/hyperion.git "$HYPERION_DIR"
 #copy LedDeviceFactory, LedDeviceTest.h/cpp to libsrc/leddevice
 #edit CMakeList.txt line 51: 
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -lwiringPi -lSerialProtocol -DRaspberryPi")
 # create and enter the build directory
 sudo mkdir "$HYPERION_DIR/build"
 cd "$HYPERION_DIR/build"
 # run cmake to generate make files on the raspberry pi
 sudo cmake ..
 # run make to build Hyperion (takes about 60min)
 sudo make
 
 How to start the new Hyperion
 #close the installed hyperion
 sudo killall hyperiond
 #copy your config to build/bin/hyperionconfig.json , in config set type to serial or sharedmemory
 #set output to device (above), rate to the serial baud if you use serial. you can use shkey for a personal shkey
 #example for serial:
 		"output"     : "/dev/ttyUSB0",
 		"rate"       : 115200,
 #start new hyperion (close with crtl+c)
 sudo bin/hyperiond bin/hyperion.config.json
 #test visualization (start ambilight on Arduino first)
 hyperion-remote --effect "Rainbow swirl fast" --duration 3000
 
 How to permanently use the new Hyperion
 # The binaries are build in "$HYPERION_DIR/build/bin". You could copy those to /usr/bin
 sudo cp ./bin/hyperion-remote /usr/bin/
 sudo cp ./bin/hyperiond /usr/bin/
 sudo cp ./bin/hyperion.config.json /etc/hyperion.config.json
 
 General information about compiling:
 If the Arduino and Raspberry program are placed in the same path make sure to name them different
 You can add other entrys to the makefile with your own sketches, or just create a new makefile
 You can also compile every sketch with the following compiler flags (install wiringPi and SerialProtocol first):
 gcc -o outpath/outname.o inpath/inname.cpp -DRaspberryPi -lwiringPi -lSerialProtocol -pedantic -Wall
 For uninstall just delete the files that the makefile writes. Havent created an uninstall yet, sorry.
 You might want to checkout the makefile in the Arduino_Serial example
 
 Direct compile and start program:
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/examples/Ambilight
 sudo gcc -o Ambilight_Serial.o Ambilight_Serial.cpp -pedantic -Wall -lwiringPi -lSerialProtocol -DRaspberryPi  && sudo ./Ambilight_Serial.o
 
 Autostart:
 sudo nano /etc/rc.local
 sudo /home/pi/Desktop/Programs/Ambilight_Serial.o </dev/null >/dev/null 2>&1 &
 
 Shared memory Information:
 If you want to use the Serial communication on the pi for other stuff like a reboot or so you need to do it different.
 Hyperion will output the Led information to a shared memory and you need to analyze this memory. This just works the same like
 the Serial mod in Hyperion, just that you need to write it again, but you can add other stuff to your program.
 Set type to "sharedMemory" and optional shkey (default: 1213)
 Check if shared memory is set: ipcs
 Nice (german) dokumentation about shared memory
 http://openbook.galileocomputing.de/unix_guru/node393.html
 
 */

// Led
const int pinLed=9;
unsigned long prevLed=0;
boolean stateLed=LOW;

// Led2
const int pinLed2=10;
unsigned long prevLed2=0;
boolean stateLed2=LOW;

// Button
const int pinButton=8;
unsigned long prevButton=0;

// LED Strip
#include "FastLED.h"
const uint16_t numLedsTV = 50;
const uint8_t dataPin = 16;
const uint8_t clockPin =15;
CRGB ledsTV[numLedsTV];

// Serial Protocol
#include <SerialProtocol.h>
// Data Stream Addresses
const int addressAmbilight = 1;
// General Commands
const int commandPing=1;
const int commandPong=2;
// Raspberry Commands
const int commandShutdown=3;
const int commandReboot=4;
// Ambilight Commands
const int commandAmbilightOn=5;
const int commandAmbilightOff=6;
const int commandAmbilightUpdate=7;

// Ambilight
unsigned long prevAmbilight = 0;
boolean stateAmbilight = true;


void setup(){
  // Led
  pinMode(pinLed, OUTPUT); 

  // Led2
  pinMode(pinLed2, OUTPUT);  

  // Button
  pinMode(pinButton, INPUT);  
  digitalWrite(pinButton, HIGH);

  // Raspberry-Arduino Bridge
  // make sure not to send too long information via debug Serial 
  // or you will get corrupted data
  Serial.begin(115200); 
  Serial1.begin(115200);
  // just to ensure Serial1 is set
  Protocol.setSerial(Serial1);

  // LED Strip
  //FastLED.addLeds<LPD8806, dataPin, clockPin, GRB>(leds, numLeds);
  FastLED.addLeds<WS2801, RGB>(ledsTV, numLedsTV); //Spi

  // Startup message
  Serial.println("Ambilight startup");

  // Ambilight
  ambilightOff();
}

void loop(){
  // Led check
  if(stateLed==HIGH && millis()-prevLed>=200){
    stateLed=LOW;
    digitalWrite(pinLed, stateLed);
  }

  // Led2 check
  if(stateLed2==HIGH && millis()-prevLed2>=500){
    stateLed2=LOW;
    digitalWrite(pinLed2, stateLed2);
  }

  // Button to send ping + turn on/off Ambilight
  if(digitalRead(pinButton)==LOW && millis()-prevButton>=200){
    ledOn();
    Serial.println("Button");

    // send Ping
    Protocol.sendCommand(commandPing);

    // change to on/off
    if(stateAmbilight==true) ambilightOff();
    else ambilightOn();

    prevButton=millis(); //for debounce
  }


  // check ambilight timeout if program suddenly
  // exits without any off signal.
  if(stateAmbilight==true && millis()-prevAmbilight>=5000){
    Serial.println("Ambilight Timeout");
    ambilightOff();
  }

}

void ledOn(void){
  digitalWrite(pinLed,HIGH);
  stateLed=HIGH;
  prevLed=millis();
}

void led2On(void){
  digitalWrite(pinLed2,HIGH);
  stateLed2=HIGH;
  prevLed2=millis();
}


// Called every time there is Serial1 input
void serialEvent1(){
  // Do NOT print in this function
  checkProtocol(Serial1);
}

void checkProtocol(Stream &stream){

  // Sets Stream to read from
  Protocol.setSerial(&stream);
  Protocol.read();

  //================================================================================
  //Check Errors
  //================================================================================

  if(Protocol.getErrorLevel()){
    // Avoid printing in the interrupt or the programm can crash
    //Serial.print("Err: ");
    //Serial.println(Protocol.getErrorLevel());
    led2On();
  }

  //================================================================================
  //Check Data
  //================================================================================

  switch(Protocol.getAddress()){
  case 0:// No Address
    break;

  case addressAmbilight:
    // writes led information to the strip if enabled
    ambilightInput(Protocol.getData());
    break;

  default :
    led2On();
    break;
  }//end switch

  //================================================================================
  //Check Commands
  //================================================================================

  switch(Protocol.getCommand()){
  case 0:// No Command
    break;

  case commandPing:
    ledOn();
    Protocol.sendCommand(commandPong);
    break;

  case commandPong:
    ledOn();
    break;

    // shutdown commands are only needed if you want that a 3rd device can
    // send a shutdown request to this Arduino. This Arduino only forwards the command
  case commandShutdown:
    Protocol.setSerial(Serial1);
    Protocol.sendCommand(commandShutdown);
    break;

  case commandReboot:
    Protocol.setSerial(Serial1);
    Protocol.sendCommand(commandReboot);
    break;

  case commandAmbilightOn:
    ledOn();
    ambilightOn();
    break;

  case commandAmbilightOff:
    ledOn();
    ambilightOff();
    break;

  case commandAmbilightUpdate:
    ambilightUpdate();
    break;

  default:
    led2On();
    break;
  }//end switch
}


void ambilightInput(uint32_t input){
  if(ambilightIsOff()) return;

  //4byte: red, green, blue, led number,
  uint8_t number = input&0xFF;
  uint32_t rgb =input>>8;

  // write data to the led array (ensure that there is no more data than we have in our strip)
  if(number<numLedsTV) ledsTV[number]= rgb;
  else led2On();
}

void ambilightUpdate(void){
  if(ambilightIsOff()) return;
  FastLED.show();
  prevAmbilight=millis();
}

boolean ambilightIsOff(void){
  if(stateAmbilight==false) {
    // we have turned ambilight off. this is an error or just the rest of the buffer
    led2On();
    Protocol.setSerial(Serial1);
    Protocol.sendCommand(commandAmbilightOff);
    return true;
  }
  return false;
}

void ambilightOff(void){
  //Serial.println("Ambilight off");
  FastLED.clear();
  ambilightUpdate();
  stateAmbilight=false;
  Protocol.setSerial(Serial1);
  Protocol.sendCommand(commandAmbilightOff);
}

void ambilightOn(void){
  if(stateAmbilight==true) {
    //Serial.println("Already on");
    return;
  }
  //Serial.println("Ambilight on!");
  stateAmbilight=true;
  ambilightUpdate();
  Protocol.setSerial(Serial1);
  Protocol.sendCommand(commandAmbilightOn);
}