/*
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
*/

#ifndef NICOHOODPROTOCOL_h
#define NICOHOODPROTOCOL_h

#include <stdint.h> //uint_t definitions
#include <stdbool.h> //bool type

//================================================================================
//Settings
//================================================================================

// empty

//================================================================================
//Raspberry only
//================================================================================

#ifdef RaspberryPi

//wiring Pi
#include <wiringPi.h>
#include <wiringSerial.h>

//Devices, check with ~ls /dev/tty*
#define ARDUINO_UNO "/dev/ttyACM0"
#define ARDUINO_MEGA "/dev/ttyACM0"
#define ARDUINO_MICRO "/dev/ttyACM0"
#define ARDUINO_PRO_MICRO "/dev/ttyACM0"
#define ARDUINO_LEONARDO "/dev/ttyACM0"
#define FTDI_PROGRAMMER "/dev/ttyUSB0"
#define ARDUINO_PRO_MINI "/dev/ttyUSB0"
// never tested, seems to be for the gpios
#define HARDWARE_UART "/dev/ttyAMA0"

//================================================================================
//Arduino only
//================================================================================

#elif defined ARDUINO


//Arduino Library
#include <Arduino.h>

#endif //#ifdef RaspberryPi

//================================================================================
//Definitions
//================================================================================

// ErrorLevel
#define NHP_MASK_INPUT		0x0F
#define NHP_INPUT_NO		0x00
#define NHP_INPUT_NEW		0x01
#define NHP_INPUT_ADDRESS	0x02
#define NHP_INPUT_COMMAND	0x04
#define NHP_INPUT_RESET		0x08
#define NHP_MASK_ERR		0xF0
#define NHP_ERR_NO			0x00
#define NHP_ERR_READ		0x10
#define NHP_ERR_END			0x20
#define NHP_ERR_DATA		0x40
#define NHP_ERR_LEAD		0x80
#define NHP_ERR_LIMIT		20	 //0-255, only for the user function

// Start Mask
#define NHP_MASK_START		0xC0 //B11|000000 the two MSB bits
#define NHP_MASK_LEAD		0xC0 //B11|000000
#define NHP_MASK_DATA		0x00 //B0|0000000 only the first MSB is important
#define NHP_MASK_END		0x80 //B10|000000

// Content Mask
#define NHP_MASK_LENGTH		0x38 //B00|111|000
#define NHP_MASK_COMMAND	0x0F //B0000|1111
#define NHP_MASK_DATA_7BIT	0x7F //B0|1111111
#define NHP_MASK_DATA_4BIT	0x0F //B0000|1111
#define NHP_MASK_DATA_3BIT	0x07 //B00000|111
#define NHP_MASK_ADDRESS	0x3F //B00|111111

// Reserved Addresses
#define NHP_ADDRESS_CONTROL 0x01

// Reserved Usages
#define NHP_USAGE_ARDUINOHID 0x01

//================================================================================
//Protocol_ Class
//================================================================================

class NHProtocol{
private:
	// Fully read data
	uint8_t mCommand;
	uint8_t mAddress;
	uint32_t mData;
	uint8_t mErrorLevel;

	// in progress reading data
	uint8_t mBlocks;
	uint32_t mWorkData;
public:
	// Constructor
	inline NHProtocol(){
		// initialize variables. mWorkData doesnt need to be initialized
		// it will be initialized while reading
		mData=0;
		mAddress=0;
		mCommand=0;
		mErrorLevel=0;
		mBlocks=0;
		readlength=0;
		writelength=0;
	}

	// access for the variables
	inline uint8_t  getCommand()   { return mCommand;    }
	inline uint8_t  getAddress()   { return mAddress;    }
	inline uint32_t getData()      { return mData;       }
	inline uint16_t getChecksumData() { return mData;	 }
	inline uint8_t  getChecksumData0() { return mData;	 }
	inline uint8_t  getChecksumData1() { return mData>>8;}
	inline uint8_t  getErrorLevel(){ return mErrorLevel; }

	// buffer for read/write operations
	uint8_t readbuffer[6];
	uint8_t readlength;
	uint8_t writebuffer[6];
	uint8_t writelength;
	inline void resetreadbuffer(){ 
		while(readlength){
			readlength--;
			readbuffer[readlength]=0;
		}
	}
	inline void resetwritebuffer() { 
		while(writelength){
			writelength--;
			writebuffer[writelength]=0;
		}
	}

	// general multifunctional read/write functions
	inline void reset(void){ mErrorLevel=NHP_INPUT_RESET; }
	bool read(uint8_t input);
	bool readChecksum(uint8_t input);
	void writeCommand(uint8_t command);
	void writeAddress(uint8_t address, uint32_t data);
	void writeChecksum(uint8_t address, uint16_t data);

	//================================================================================
	//Functions for easy user interaction (end of the main Protocol implementation)
	//You can implement similar functions for your specific data transfer method.
	//This is more thought as an example and for easy use. Thatswhy inline.
	//================================================================================

#ifdef RaspberryPi
	// set filedescriptor
	inline bool read(int &fd){
		// check if fd has changed and reset
		static int prevfd=-1;
		if(fd<0)return false;
		if(fd!=prevfd){
			prevfd=fd;
			reset();
		}
		uint8_t errCount=0;
		// search until there is a valid input
		while(fd>=0 && serialDataAvail(fd)){
			if(read(serialGetchar(fd))) return true;
			// stop if there are too many errors to escape the loop
			// this can cause errors if you dont check stream->available
			// outside this function. Thatswhy a high limit is recommended
			if(getErrorLevel() & NHP_MASK_ERR) errCount++;
			if(errCount>NHP_ERR_LIMIT) break;
		}
		return false;
	}

	inline int8_t sendAddress(uint8_t a, uint32_t d, int &fd){
		writeAddress(a,d);
		for(int i=0;i<writelength;i++){
			if(fd<0) return -1;
			serialPutchar (fd, writebuffer[i]);
		}
		return writelength;
	}

	inline int8_t sendChecksum(uint8_t a, uint32_t d, int &fd){
		writeAddressChecksum(a,d);
		for(int i=0;i<writelength;i++){
			if(fd<0) return -1;
			serialPutchar (fd, writebuffer[i]);
		}
		return writelength;
	}

	inline int8_t sendCommand(uint8_t c, int &fd){
		if(fd<0) return -1;
		writeCommand(c);
		serialPutchar (fd, writebuffer[0]);
		return writelength;
	}

#elif defined ARDUINO
	inline bool read(Stream &s){ return read(&s);}
	inline bool read(Stream *pStream){
		// check if Stream has changed and reset
		// you might want to check if there is any pending reading
		// with !getErrorLevel()&NHP_MASK_INPUT
		static Stream *prevStream=NULL;
		if(pStream!=prevStream){
			prevStream=pStream;
			reset();
		}
		uint8_t errCount=0;
		// search until there is a valid input
		while(pStream->available()){
			if(read(pStream->read())) return true;
			// stop if there are too many errors to escape the loop
			// this can cause errors if you dont check stream->available
			// outside this function. Thatswhy a high limit is recommended
			if(getErrorLevel() & NHP_MASK_ERR) errCount++;
			if(errCount>NHP_ERR_LIMIT) break;
		}
		return false;
	}

	inline uint8_t sendAddress(uint8_t a, uint32_t d, Stream &s){ return sendAddress(a,d,&s);}
	inline uint8_t sendAddress(uint8_t a, uint32_t d, Stream *pStream){
		writeAddress(a,d);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

	inline uint8_t sendChecksum(uint8_t a, uint16_t d, Stream &s){ return sendChecksum(a,d,&s);}
	inline uint8_t sendChecksum(uint8_t a, uint16_t d, Stream *pStream){
		writeChecksum(a,d);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

	inline uint8_t sendCommand(uint8_t c,Stream &s){ return sendCommand(c,&s);}
	inline uint8_t sendCommand(uint8_t c, Stream *pStream){
		writeCommand(c);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

#endif
};

extern NHProtocol NHP;
extern NHProtocol Protocol;

#endif
