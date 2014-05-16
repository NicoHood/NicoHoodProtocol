SerialProtocol
==========

A Protocol for Arduino - Raspberry Pi communication

*Click Download as zip to download or this link*
https://github.com/NicoHood/SerialProtocol/archive/master.zip

Dokumentation:
===========
http://nicohood.wordpress.com/2014/04/18/arduino-raspberry-pi-serial-communication-protocol-via-usb-and-cc/
and in local folder ./dokumentation a copy of the blogpost

Questions? Just ask under my blog entry/message me there.


Installation on Raspberry Pi
====================
This Part is only important for the Raspberry Pi, if you want to use it there.
First navigate to the directory where you downloaded the library. And then run the install command.
Now you are able to use the #include <NicoHoodProtocol.h> global in every program.

    $ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
    $ sudo make install
 
 Compile you program of choice:

    $ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
    $ sudo make serialtest
    $ sudo make ambilight
 
 Start your program of choice (close with crtl+c):

    $ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
    $ sudo examples/Arduino_Serial/Pi_Serial.o
    $ sudo examples/Ambilight_Serial/Ambilight_Serial.o

See the examples for further information how to setup a Serial on the Raspberry Pi.


 Ambilight Instructions
================

In order to use the ambilight sketches you need to use hyperion as ambilight program and also modify it.
You can use the precompiled hyperion version and just install it or compile the sources if you like to.
**The library needs do be installed for this.**


 *Find Serial device on Raspberry with ~ls /dev/tty*
 ARDUINO_UNO "/dev/ttyACM0"
 FTDI_PROGRAMMER "/dev/ttyUSB0"
 HARDWARE_UART "/dev/ttyAMA0"

 Additional: How to compile Hyperion
-----------------------------------------------------
 How to compile the new Hyperion:

```bash
$ sudo apt-get update
$ sudo apt-get install git cmake build-essential libprotobuf-dev libQt4-dev libusb-1.0-0-dev protobuf-compiler python-dev
$ cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/

# create hyperion directory and checkout the code from github
$ export HYPERION_DIR="hyperion"
$ sudo git clone https://github.com/tvdzwan/hyperion.git "$HYPERION_DIR"

# copy LedDeviceFactory, LedDeviceTest.h/cpp to libsrc/leddevice
$ sudo cp HyperionSource/LedDevice* $HYPERION_DIR/libsrc/leddevice/
# edit CMakeList.txt line 51 or copy the pre edited file: 
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -lwiringPi -lNicoHoodProtocol -DRaspberryPi")
$ sudo cp HyperionSource/CMakeList.txt  $HYPERION_DIR/

# create and enter the build directory
$ sudo mkdir "$HYPERION_DIR/build"
$ cd "$HYPERION_DIR/build"
# run cmake to generate make files on the raspberry pi
sudo cmake ..
# run make to build Hyperion (takes about 60min)
sudo make
 ```

 How to start the new Hyperion

    #close the installed hyperion
    $ sudo killall hyperiond
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
 
 

Explanation
========

```bash
var s = "JavaScript syntax highlighting";
alert(s);
alert(s);
alert(s);
alert(s);
```
 
```python
s = "Python syntax highlighting"
print s
```
 
```
No language indicated, so no syntax highlighting. 
But let's throw in a <b>tag</b>.
```


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
==========================================================


|11LLLDDD|0DDDDDDD|0DDDDDDD|0DDDDDDD|0DDDDDDD|10AAAAAA|


| Tables        | Are           | Cool  |
| ------------- |:-------------:| -----:|
| col 3 is      | right-aligned | $1600 |
| col 2 is      | centered      |   $12 |
| zebra stripes | are neat      |    $1 |


==========================================================
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

The Protocol knows at any time if its a lead/data/end block.
It will detect Protocol syntax errors in ErrorLevel (see notes below).
If you send other stuff through the stream you can filter out Protocol
data for example. Positiv is that: 0 and 255 is always invalid on its own.
Ascii letters are also invalid on their own and fast recognized.
If you strictly want to filter out Protocol data i recommend to send an inverse
of two bytes (see user example below).

ErrorLevel:
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
*/


Versions:
==============

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
    

[Readme written with this markdown preview](http://tmpvar.com/markdown.html)