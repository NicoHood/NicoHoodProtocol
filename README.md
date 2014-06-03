NicoHoodProtocol
================

A Protocol for Arduino-RaspberryPi, Arduino-Arduino and other communication.

This Protocol is designed to send up to 4 bytes through a 1 byte data pipe
like a serial communication with a clear start/end indication. It supports up to 64 addresses 
and up to 16 direct commands at the moment. See details below.

The Protocol is written for a general use but also has built in functions to use it with
Arduino and RaspberryPi.

Functions/Projects:

* Communicate via Serial, CDCSerial, I2C and every other 1 byte communication with this compact protocol
* Let your Arduinos and Raspberrys easily interact
* Use the Protocol to send Ambilight data to your Arduino and use any led strip you want
* The Arduino Hoodloader makes use of the library too
* C++ and C library provided

Download
========

To download and install the library just hit the download button somewhere on the right.
Make sure to rename the folder (remove master). See Arduino.cc for help.
For installation on the Raspberry Pi see instructions below.

Documentation:
==============

For any documentation see this readme file, check out the examples or have a look at my blog
where I explain the examples and show off other stuff.
http://nicohood.wordpress.com/

Questions? Just ask under my blog entry or message me there.

Explanation of the Protocol
===========================

The Protocol is made to send up to 4 bytes as compact as possible.
They are packed into the Protocol and the result is written in a writebuffer
that can have up to 6 bytes. These bytes are called blocks.
It will leave out all zeros befor the MSB. In the chart below you can see how many bytes
you can send with how many blocks. #0 is the LSB and #31 the MSB.

For the protocol I do a little trick here:
If it needs to send 6 blocks it save the MSB #31 of the data in the length.
This works, because the length is never more than 6 (see chart below).
The reader has to decode this of course. Same works for the 4 bit command.

Address (2-6 blocks) or Command (1block):
=========================================

*Address 1 is reserved for special Control Address like for ArduinoHID.
Please do not use Address 1 to not get in any conflict*

| Lead     | Data3    | Data2    | Data1    | Data0    | End      |
|:--------:|:--------:|:--------:|:--------:|:--------:| :-------:|
| 11LLLDDD | 0DDDDDDD | 0DDDDDDD | 0DDDDDDD | 0DDDDDDD | 10AAAAAA |

```
Lead: 2bit lead indicator(11), 3bit length (including data bit #31, command bit #3), 3bit data/3bit command
Data: 1bit data indicator(0) , 7bit optional data (0-4 blocks)
End : 2bit end indicator (10), 6bit address

3bit length in lead explained:
command  00(0) 4bit command + bit #3 is zero
command  00(1) 4bit command + bit #3 is one
0-3   =2 010   2 blocks (3bit)
4-10  =3 011   3 blocks (10bit)
11-17 =4 100   4 blocks (17bit)
18-24 =5 101   5 blocks (24bit)
25-31 =6 11(0) 6 blocks (32bit) + bit #31 is zero
32    =7 11(1) 6 blocks (32bit) + bit #31 is one
```

Address with Checksum (only looking at the 4 bytes):
=========================================
| Byte3    | Byte2    | Byte1    | Byte0    |
|:--------:|:--------:|:--------:|:--------:|
| ~Data1   | ~Data0   | Data1    | Data0    |

```
Data1 and Data0 can be read as uint8_t each with
getChecksumData0
getChecksumData1
Data1 and Data0 can be read as uint16_t with
getChecksumData

For the Control Address (Address1) Data1 contains the usage and Data0 information related to the usgae.

```

The Protocol knows at any time if its a lead/data/end block.
It will detect Protocol syntax errors in ErrorLevel (see notes below).
If you send other stuff through the stream you can filter out Protocol
data for example. Positiv is that: 0 and 255 is always invalid on its own.
Ascii letters are also invalid on their own and fast recognized.
If you strictly want to filter out Protocol data i recommend to send an inverse
of two bytes (see user example below).

ErrorLevel:
```
==========
|76543210|
==========
0-3: Input indicators or intern reset trigger, 0 is set if 1 or 2 is set
0: NHP_INPUT_NEW
1: NHP_INPUT_ADDRESS
2: NHP_INPUT_COMMAND
3: NHP_INPUT_RESET
4-7: Error indicator, 4 is set if 5,6 or 7 is set
4: NHP_ERR_READ
5: NHP_ERR_END
6: NHP_ERR_DATA
7: NHP_ERR_LEAD
```
*/

Project: Installation on Raspberry Pi
============================
This part is only important for the Raspberry Pi, if you want to use it there.

```bash
#Before you can use the library on Raspberry Pi with the Serial interface
#You need to install WiringPi:
$ sudo apt-get install git-core
$ sudo apt-get update
$ sudo apt-get upgrade
$ cd /tmp && git clone git://git.drogon.net/wiringPi
$ cd wiringPi && ./build
#Test installation:
$ gpio -v
$ gpio readall

#Then install the Protocol library
#First navigate to the directory where you downloaded the library.
#And then run the install command.
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
$ sudo make install

#Compile you program of choice:
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
$ sudo make serialtest
$ sudo make ambilight

#Start your program of choice (close with crtl+c):
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
$ sudo examples/Arduino_Serial/Pi_Serial.o
$ sudo examples/Ambilight_Serial/Ambilight_Serial.o
```

Now you are able to use the #include <NicoHoodProtocol.h> global in every program.
See the examples for further information how to setup a Serial on the Raspberry Pi.


Project: Ambilight Instructions
======================

In order to use the ambilight sketches you need to use hyperion as ambilight program and also modify it.
[**You need to install Hyperion first on your own.**]
(https://github.com/tvdzwan/hyperion/wiki/installation)
You can use the pre compiled hyperion version and copy the two files to the system path. 
You can also recompile the sources if you want to(see instruction below).

Edit the hyperion config first. Set the device type to "serial" (default) or "shared memory" (advanced).
Set baud to 115200 and for output check this:

*Find Serial device on Raspberry with ~ls /dev/tty*
```
ARDUINO_UNO "/dev/ttyACM0"
ARDUINO_MEGA "/dev/ttyACM0"
ARDUINO_MICRO "/dev/ttyACM0"
ARDUINO_PRO_MICRO "/dev/ttyACM0"
ARDUINO_LEONARDO "/dev/ttyACM0"
FTDI_PROGRAMMER "/dev/ttyUSB0"
ARDUINO_PRO_MINI "/dev/ttyUSB0"
// never tested, seems to be for the gpios
HARDWARE_UART "/dev/ttyAMA0"
```

Example config:
```
	"device" :
	{
		"name"       : "MyPi",
		"type"       : "serial",
		"output"     : "/dev/ttyACM0",
		"rate"       : 115200,
		"colorOrder" : "rgb"
	},
```
The highest baud is to ensure that there is no lagg. For the Micro/Leonardo the baud doesnt care because
its a CDC Serial (if you dont know what this is, just ignore). For now it seems that the Micro/Leonardo
cant handle ambilight on the CDC (usb) Serial, because the implemenation is too slow. But you can use Serial1
on the Micro/Leonardo and a ftdi programmer to get to usb (connect rx-tx, tx-rx, gnd-gnd). It works fine with
an Arduino Uno/Mega.

I also recommend to set the bootsequence duration to 8000, because the Serial initialization takes some time. 

```bash
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
$ sudo make hyperionmod
$ sudo make hyperionconfig
```

 Additional: How to compile Hyperion
====================================
**The NicoHoodProtocol and WiringPi library needs do be installed for this.
This is only if you want to know how i did this any nobody really needs to know this.**

How to compile the new Hyperion:

```bash
$ sudo apt-get update
$ sudo apt-get install git cmake build-essential libprotobuf-dev libQt4-dev libusb-1.0-0-dev protobuf-compiler python-dev
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/

# create hyperion directory and checkout the code from github
$ export HYPERION_DIR="hyperion"
$ sudo git clone https://github.com/tvdzwan/hyperion.git "$HYPERION_DIR"

# copy new sources to the hyperion. If they change anything you might want to
# compare the two sources and add the code. otherwise it wont compile.
# copy LedDeviceFactory, LedDeviceTest.h/cpp to libsrc/leddevice
$ sudo cp HyperionSource/LedDevice* $HYPERION_DIR/libsrc/leddevice/
# edit CMakeList.txt line 51 or copy the pre edited file: 
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -lwiringPi -lNicoHoodProtocol -DRaspberryPi")
$ sudo cp HyperionSource/CMakeLists.txt  $HYPERION_DIR/CMakeLists.txt

# create and enter the build directory
$ sudo mkdir "$HYPERION_DIR/build"
$ cd "$HYPERION_DIR/build"
# run cmake to generate make files on the raspberry pi
sudo cmake ..
# run make to build Hyperion (takes about 60min for the first compiling)
sudo make
 ```

 How to start the new Hyperion
```bash
#close the installed hyperion
$ sudo killall hyperiond

#copy your config to build/bin/hyperionconfig.json , in config set type to "serial" or "sharedmemory"
#set output to device (above), rate to the serial baud if you use serial. you can use "shkey" for a personal shkey
#see example above
$ sudo cp ../../HyperionSource/hyperion.config.json bin/hyperion.config.json

#start new hyperion (close with crtl+c)
$ sudo bin/hyperiond bin/hyperion.config.json
#test visualization in a new terminal(start ambilight on Arduino first)
$ hyperion-remote --effect "Rainbow swirl fast" --duration 3000
```

How to permanently use the new Hyperion
```bash
# The binaries are build in "$HYPERION_DIR/build/bin". You could copy those to /usr/bin
$ sudo cp ./bin/hyperion-remote /usr/bin/
$ sudo cp ./bin/hyperiond /usr/bin/
$ sudo cp ./bin/hyperion.config.json /etc/hyperion.config.json
```

For developing new code you can edit the sources in HyperionSource and do in the main path
```bash
$ sudo make hyperioncopysource
$ sudo ./hyperion/build/make
$ sudo make hyperiongetsource
$ sudo make hyperionmod
$ sudo make hyperionconfig
$ sudo killall hyperiond
$ sudo /usr/bin/hyperiond /etc/hyperion.config.json </dev/null >/dev/null 2>&1 &
$ sudo hyperion-remote --effect "Rainbow swirl fast" --duration 3000
```

General information about compiling:
If the Arduino and Raspberry program are placed in the same path make sure to name them different (Pi_programm.cpp, Arduino_programm.ino).
You can add other entrys to the makefile with your own sketches, or just create a new makefile
You can also compile every sketch with the following compiler flags (install wiringPi and NicoHoodProtocol first):
gcc -o outpath/outname.o inpath/inname.cpp -DRaspberryPi -lwiringPi -lNicoHoodProtocol -pedantic -Wall
For uninstall just delete the files that the makefile writes. Havent created an uninstall yet, sorry.
For reinstall just use the install again.
You might want to checkout the makefile in the Arduino_Serial example

Under Construction
------------------

 Direct compile and start program:
 cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/examples/Ambilight
 sudo gcc -o Ambilight_Serial.o Ambilight_Serial.cpp -pedantic -Wall -lwiringPi -lSerialProtocol -DRaspberryPi  && sudo ./Ambilight_Serial.o
 
 Autostart:
 sudo nano /etc/rc.local
 sudo /home/pi/Desktop/Programs/Ambilight_Serial.o </dev/null >/dev/null 2>&1 &
 
Shared memory Information:
--------------------------
If you want to use the Serial communication on the pi for other stuff like a reboot or so you need to do it different.
Hyperion will output the Led information to a shared memory and you need to analyze this memory. This just works the same like
the Serial mod in Hyperion, just that you need to write it again, but you can add other stuff to your program.
Set type to "sharedMemory" and optional shkey (default: 1213)
Check if shared memory is set: $ ipcs
Nice (german) dokumentation about shared memory
http://openbook.galileocomputing.de/unix_guru/node393.html


Version History
===============
```

1.6 (todo)
+ rework the examples and check for syntax
+ detailed blog articles about the examples
- remove the old Protocol Class

1.5 (03.06.2014)
+ added c support (made for Hoodloader Project)
+ added Project Hoodloader
+ added keywords.txt definitions
+ added licence
+ bugfixes

1.4 (17.5.2014)
+ changed the syntax a bit again
+ made the library more general to use with other stuff too
+ changed inner sturcture + read/write functions
+ renamed the library
+ added some proper examples for Ambilight
+ added readme information

1.3 Never really official released

1.2 (xx.05.2014)
+ added hyperion mod and example sketch
+ added Errorlevel
+ changed overall structure
+ changed writing function to suit for Wire library
+ added easy to use makefile with installation
- removed outdated sketches

1.1 (19.04.2014)
+ Added Command function, changed syntax, cleared code
+ Updated Examples, edited makefile
- Removed direct Serial access
- Removed printing functions

1.0 Release (18. 4.2014)

```

Known Bugs
==========
None - yey!!!

Licence and Copyright
=====================
Daniel Garcia from the FASTLED library helped me with this code first.
If you use this library for any cool project let me know!

```
Copyright (c) 2014 NicoHood
See the readme for credit to other people.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
