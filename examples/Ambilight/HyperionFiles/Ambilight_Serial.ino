/*
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
 # create and enter the build directory
 sudo mkdir "$HYPERION_DIR/build"
 cd "$HYPERION_DIR/build"
 # run cmake to generate make files on the raspberry pi
 sudo cmake ..
 # run make to build Hyperion (takes about 30min)
 sudo make

 How to start the new Hyperion
 #close the installed hyperion
 sudo killall hyperiond
 #copy your config to bin/hyperionconfig.json , in config set type to serial or sharedmemory
 #set output to device (above), rate to the serial baud if you use serial. you can use shkey for a personal shkey
 #start new hyperion (close with crtl+c)
 sudo bin/hyperiond bin/hyperion.config.json
 #test visualization (start ambilight on Arduino first)
 hyperion-remote --effect "Rainbow swirl fast" --duration 3000

 How to permanently use the new Hyperion
 # The binaries are build in "$HYPERION_DIR/build/bin". You could copy those to /usr/bin
 sudo cp ./bin/hyperion-remote /usr/bin/
 sudo cp ./bin/hyperiond /usr/bin/
 
 */

//led+button
const uint8_t ledPin=9;
const uint8_t buttonPin=8;
unsigned long lastButton=0; //debounce

//LPD8806 LED Strip
#include "FastLED.h"
const uint16_t numLeds = 50;  // How many leds in your strip?
const uint8_t dataPin = 16;
const uint8_t clockPin =15;
CRGB leds[numLeds]; // Define the array of leds

//Raspberry-Arduino Bridge
#include "SerialProtocol.h"
const int commandPing=1;
const int commandPong=2;

//Ambilight
const uint8_t addressAmbilight = 1;
const uint8_t commandOn=3;
const uint8_t commandOff=4;
const uint8_t commandUpdate=5;
int lastnumber = 0; //needs to be int for setting -1
boolean update = false;
boolean status = false;
unsigned long lastAmbilight = 0;

void setup(){
  // Raspberry-Arduino Bridge
  // Serial + Serial1 begin
  // make sure not to send too long information via debug Serial 
  // or you will get corrupted data
  Serial.begin(115200); 
  Serial1.begin(115200);
  Protocol.setSerial(Serial1);

  //Button + Led setup
  pinMode(ledPin, OUTPUT);      
  pinMode(buttonPin, INPUT);  
  digitalWrite(buttonPin, HIGH);

  //LED Strip
  //FastLED.addLeds<LPD8806, dataPin, clockPin, GRB>(leds, numLeds);
  FastLED.addLeds<WS2801, RGB>(leds, numLeds); //Spi

  //startup message, default turned off
  Serial.println("Ambilight startup");
  ambilightoff();
}

void loop(){

// button to send ping + turn on/off Ambilight
  if(digitalRead(buttonPin)==LOW && millis()-lastButton>=200){
    Serial.println("Button");

    // send Ping
    Protocol.sendCommand(commandPing);

    // change to on/off
    if(status==true) ambilightoff();
    else ambilighton();

    lastButton=millis(); //for debounce
  }

  // read Serial
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

  // check for new Data
  switch(Protocol.getAddress()){
  case 0:
    // No Address
    break;
  case addressAmbilight:
    {
      // only visualize if its on. This is to clear the pending Serial buffer
      // to prevent sending colors after turning off with a button
      // it might take some time for the ambilight host to recognize the turn off signal
      // this is also to ignore any input if something goes wrong
      if(status==true){
	  //4byte: led number, red, green, blue
        uint32_t input = Protocol.getData();
        uint8_t number = input&0xFF;
        input>>=8;
        uint8_t blue = input&0xFF;
        input>>=8;
        uint8_t green = input&0xFF;
        input>>=8;
        uint8_t red = input&0xFF;

        // write data to the led array (ensure that there is no more data than we have in our strip)
        if(number<numLeds){
          leds[number]= CRGB(red,green, blue);
          // ensures to write the leds if the update was missed
          if(number<=lastnumber) ambilightupdate();
          lastnumber=number;
        }
		// else ignore that stuff. You can add a print here.
      }
	  // we have turned ambilight off. this is an error or just the rest of the buffer
      else Serial.println("Already off");
    }
    break;
  default :
    //debug Print, add other functions here
    Serial.print(Protocol.getData(),HEX);
    Serial.print(", ");
    Serial.print(Protocol.getAddress(),HEX);
    Serial.print("\n");  
    break;
  }//end switch

  // check for new commands
  switch(Protocol.getCommand()){
  case 0:
    // No Command
    break;
  case commandPing:
    Serial.println("Ping!");
	// send Pong back
    Protocol.sendCommand(commandPong);
    break;
  case commandPong:
    Serial.println("Pong!");
    break;
  case commandOn:
    ambilighton();
    break;
  case commandOff:
    ambilightoff();
    break;
  case commandUpdate:
    ambilightupdate();
	// to not update again if we recieve the next value
	// this is only because this sketch should work 100%
	// normally you can leave the whole lastnumber stuff away.
	// an update shouldnt be missed, and even if it gets missed
	// the next update will be send soon.
    lastnumber=-1;
    break;
  default:
    Serial.println("Unknown Command");
    break;
  }//end switch

  // ambilight timeout if program suddenly
  // exits without any off signal.
  if(millis()-lastAmbilight>=5000) ambilightoff();

}

// Ambilight functions

void ambilightoff(void){
  Serial.println("Changed to off");
  status=false;
  FastLED.clear();
  FastLED.show();
  // send back the okay
  Protocol.sendCommand(commandOff);
}

void ambilighton(void){
  Serial.println("Changed to on");
  status=true;
  // send back the okay
  Protocol.sendCommand(commandOn);
}

void ambilightupdate(void){
  //Serial.println("update!");
  FastLED.show();
  lastAmbilight=millis();
}








