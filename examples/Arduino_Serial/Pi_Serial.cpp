/*
 Pi_Serial.cpp - SerialProtocol library - demo
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

#ifdef RaspberryPi //just that the Arduino IDE doesnt compile these files.

//include system librarys
#include <stdio.h> //for printf
#include <stdint.h> //uint8_t definitions
#include <stdlib.h> //for exit(int);
#include <string.h> //for errno
#include <errno.h> //error output

//wiring Pi
#include <wiringPi.h>
#include <wiringSerial.h>

//Protocol library
#include "../../SerialProtocol.h"
int fd; //filedescriptor
char device[]= FTDI_PROGRAMMER;
unsigned long baud = 9600;

//Setup
long previousMillis = 0; 
long previousMillis2 = 0; 
int count = 0;

//prototypes
int main(void);
void loop(void);
void setup(void);


void setup(){
  printf("%s", "Raspberry Startup!\n");
  fflush(stdout);

  //get filedescriptor
  if ((fd = serialOpen (device, baud)) < 0){
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    exit(1); //error
  }

  //setup GPIO in wiringPi mode 
  if (wiringPiSetup () == -1){
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    exit(1); //error
  }

  //pass the filedescriptor to the Protocol
  Protocol.setSerial(fd);
}


void loop(){
  // if you want to read every input this time and clear the queue
  //while(Protocol.read()){ 
  // read one input if available
  if(Protocol.read()){
    /*
    printf("%s", "Address: "); 
     fflush(stdout);
     printf("%d", Protocol.getAddress()); 
     fflush(stdout);
     printf("%s", " Data: "); 
     fflush(stdout);
     printf("%x", Protocol.getData()); 
     fflush(stdout);
     printf("%s", " Command: "); 
     fflush(stdout);
     printf("%d", Protocol.getCommand()); 
     fflush(stdout);
     printf("%s", "\n"); 
     fflush(stdout);*/

    switch(Protocol.getAddress()){
    case 0:
      // No Address -> Command
      break;
    case 1:
      printf("%s", "Random: "); 
      printf("%x", Protocol.getData()); 
      printf("%s", "\n"); 
      break;
    case 2:
      printf("%s", "Analog: "); 
      printf("%x", Protocol.getData()); 
      printf("%s", "\n"); 
      break;
    default:
      printf("%s", "Wrong Address\n"); 
      fflush(stdout);
      break;
    }//end switch

    switch(Protocol.getCommand()){
    case 0:
      // No Command -> Address
      break;
    case 1:
      printf("%s", "Ping!\n"); 
      fflush(stdout);
      break;
    case 2:
      printf("%s", "Button: ");
      fflush(stdout);
      count++;
      printf("%d" "\n", count);
      fflush(stdout);
      if(count>20) system("sudo reboot");
      break;
    default:
      printf("%s", "Wrong Command\n"); 
      fflush(stdout);
      break;
    }//end switch
  }

  // fade led every 7 sec
  if(millis() - previousMillis > 2000) {
    for(int i=0;i<256;i++){
      Protocol.sendAddress(1, i);
      //delay(100);
    }    
    previousMillis = millis();   
  }

  // send Ping every 10 sec
  if(millis() - previousMillis2 > 10000) {
    Protocol.sendCommand(1);
    previousMillis2 = millis();   
  }

}


int main() {
  setup();
  while(1) loop();
  return 0;
}

#endif //#ifdef RaspberryPi





