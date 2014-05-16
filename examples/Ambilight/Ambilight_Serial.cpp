/*
Ambilight_Serial.cpp - SerialProtocol library - Ambilight demo
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

General information about compiling:
If the Arduino and Raspberry program are placed in the same path make sure to name them different
You can add other entrys to the makefile with your own sketches, or just create a new makefile
You can also compile every sketch with the following compiler flags (install wiringPi and SerialProtocol first):
gcc -o outpath/outname.o inpath/inname.cpp -DRaspberryPi -lwiringPi -lSerialProtocol -pedantic -Wall
For uninstall just delete the files that the makefile writes. Havent created a uninstall yet, sorry.
You might want to checkout the makefile in the Arduino_Serial example

Direct compile and start program:
cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/examples/Ambilight
sudo gcc -o Ambilight_Serial.o Ambilight_Serial.cpp -pedantic -Wall -lwiringPi -lSerialProtocol -DRaspberryPi  && sudo ./Ambilight_Serial.o

Shared memory Information:
If you want to use the Serial communication on the pi for other stuff like a reboot or so you need to do it different.
Hyperion will output the Led information to a shared memory and you need to analyze this memory. This just works the same like
the Serial mod in Hyperion, just that you need to write it again, but you can add other stuff to your program.
Set type to "sharedMemory" and optional shkey (default: 1213)
Check if shared memory is set: ipcs
Nice (german) dokumentation about shared memory
http://openbook.galileocomputing.de/unix_guru/node393.html

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
#include <SerialProtocol.h>
int fd; //filedescriptor
char device[]= FTDI_PROGRAMMER;
unsigned long baud = 115200;

// shared memory
#include <sys/ipc.h>
#include <sys/shm.h>
// id, key, max leds, memory pointer, on/off status, prev led buffer, update led, ledtime
int shID;
#define SH_KEY 1213
#define SH_SIZE 255*3+1
uint8_t *shMemory;
bool status=false;
#define MAXLEDS 255
uint8_t ledBuffer[MAXLEDS*3]={0};
uint8_t updateLed=0;
unsigned long ledTime=0;

//prototypes
int main(void);
void loop(void);
void setup(void);
int sendLeds(void);


void setup(){
	printf("Raspberry Startup!\n");
	fflush(stdout);

	//get serial filedescriptor
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

	// send on request
	printf("Sending on request.\n");
	Protocol.sendCommand(3);
}


void loop(){
	// check Serial for new Commands
	while(Protocol.read()){
		switch(Protocol.getCommand()){
		case 0:
			{
				// No Command
			}
			break;
		case 1:
			{
				printf("Ping!\n");
				// send Pong
				Protocol.sendCommand(2);
			}
			break;
		case 2:
			{
				printf("Pong!\n");
			}
			break;
		case 3:
			{
				/// access shared memory
				shID = shmget(SH_KEY, SH_SIZE, 0666);
				if (shID < 0) {
					fprintf (stderr, "Unable to get shared memory: %s\n", strerror (errno)) ;
					// send off request
					printf("Sending off request.\n");
					Protocol.sendCommand(4);
					break; //error
				}
				status=true;
				printf("Ambilight set to on\n");
			}
			break;
		case 4:
			{
				status=false;
				printf("Ambilight set to off\n");
			}
			break;
		default:
			{
				//add your other command like a shutdown here
				printf("Invalid Command via Serial.\n");
			}
			break;
		} // end switch getCommand()


		switch(Protocol.getAddress()){
		case 0:
			// No Address
			break;
		default:
			printf("Wrong Address\n");
			break;
		} // end switch getAddress()

	} // end while read()

	// update leds via Serial communication
	// need a little debounce here otherwise the serial will oveflow
	// and you will onyl get corrupted data
	// also you can see in the debug output there are always some
	// times where only one led is updated. this is the one led
	// to ensure not do get a wrong outdated screen. So 40ms is fast enough.
	if(millis()-ledTime>=40){
		sendLeds();
	}

}

int sendLeds(void){
	// return if Ambilight is turned off
	if(status==false) return -1;

	// attach shared memory, read only
	shMemory = (uint8_t *)shmat(shID, 0,  SHM_RDONLY);
	if (shMemory==(uint8_t *)-1) {
		fprintf (stderr, "Unable to attach shared memory: %s\n", strerror (errno));
		// send on request
		printf("Sending off request.\n");
		Protocol.sendCommand(4);
		status=false;
		return -1; //error
	} 

	// get Led number
	uint8_t size = shMemory[0];

	// check led size
	if (size==0) {
		printf("Leds size is zero\n");
		return -1;
	}
	else if(size>MAXLEDS){
		printf("Invalid attempt to write led values. Not more than %d leds allowed.\n" , MAXLEDS);
		return -1;
	}

	// write the leds to the Serial
	uint8_t updateCount=0;
	for (uint8_t i=0; i<size; i++)	{
		// temporary helper values to compare new leds with the old buffer leds
		// one offset for length
		uint8_t red   = shMemory[1+i*3];
		uint8_t green = shMemory[1+i*3+1];
		uint8_t blue  = shMemory[1+i*3+2];

		uint8_t buffred   = ledBuffer[i*3];
		uint8_t buffgreen = ledBuffer[i*3+1];
		uint8_t buffblue  = ledBuffer[i*3+2];

		// only write updated leds to the serial to speed up/ dont use too much serial buffer
		// problem is that the receiver might miss some constant, not updated data over the time
		// this might be a problem for sending everything black
		// therefore we update at least one led for every update.
		if(buffred!=red || buffgreen!=green || buffblue!=blue || i==updateLed){
			uint32_t colors = (red<<(8*3)) + (green<<(8*2)) + (blue<<8) + i ;
			Protocol.sendAddress(1, colors);
			updateCount++;
		}
		// write previous color data to the buffer
		ledBuffer[i*3] = red;
		ledBuffer[i*3+1] = green;
		ledBuffer[i*3+2] = blue;
	}

	// debug output
	printf("Updating %d of %d Leds.\n", updateCount, size);

	// next time update the next Led in the strip
	updateLed++;
	if(updateLed>=size) updateLed=0;

	// Write an Update Command to the Serial to update leds
	Protocol.sendCommand(5);

	// detach shared memory
	shmdt(shMemory);

	// save last written Leds
	ledTime=millis();
	return 0;
}


int main() {
	setup();
	while(1) loop();
	return 0;
}

#endif //#ifdef RaspberryPi
