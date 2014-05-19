/*
NicoHoodProtocol.h - NicoHoodProtocol library - description
Copyright (c) 2014 NicoHood.  All right reserved.
Daniel Garcia from the FASTLED library helped me with this code
*/

#ifndef NICOHOODPROTOCOL_h
#define NICOHOODPROTOCOL_h

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

//include system librarys
#include <stdint.h> //uint8_t definitions

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

#else //Arduino

//Arduino Library
#include <Arduino.h>

#endif //#ifdef RaspberryPi

//================================================================================
//Explanation
//================================================================================

/*
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
|11LLLDDD||0DDDDDDD|0DDDDDDD|0DDDDDDD||0DDDDDDD||10AAAAAA|
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

	// general multifunctional read/write functions
	bool read(uint8_t input);
	inline void reset(void){ mErrorLevel=NHP_INPUT_RESET; }
	void write(uint8_t command);
	void write(uint8_t address, uint32_t data);

	// buffer for read/write operations
	uint8_t readbuffer[6];
	uint8_t readlength;
	inline void resetreadbuffer() { readlength=0;}
	uint8_t writebuffer[6];
	uint8_t writelength;
	inline void resetwritebuffer() { writelength=0;}

	// access for the variables
	inline uint8_t  getCommand()   { return mCommand;    }
	inline uint8_t  getAddress()   { return mAddress;    }
	inline uint32_t getData()      { return mData;       }
	inline uint16_t getChecksumData() { return mData;	 }
	inline uint8_t  getChecksumData0() { return mData;	 }
	inline uint8_t  getChecksumData1() { return mData>>8;}
	inline uint8_t  getErrorLevel(){ return mErrorLevel; }

	//================================================================================
	//Functions for easy user interaction (end of the main Protocol implementation)
	//You can implement similar functions for your specific data transfer method.
	//This is more thought as an example and for easy use. Thatswhy inline.
	//================================================================================

	// reads two bytes and check its inverse
	inline bool readChecksum(uint8_t input){
		if(read(input)){
			// if there is an address input (comand invalid, too insecure)
			if(getAddress() && (((getData()&0xFFFF) ^ (getData()>>16))==0xFFFF)){
				// make sure to use getAddress() and getData()&0xFFFF
				return true;
			}
			// else you can forward the buffer or pass -1 as error
		}
		return false;
	}
	// write two bytes with its inverse
	inline void writeChecksum(uint8_t address, uint16_t data){
		uint32_t temp=~data;
		uint32_t checksum=(temp<<16)|data;
		write(address,checksum);  
	}

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
		write(a,d);
		for(int i=0;i<writelength;i++){
			if(fd<0) return -1;
			serialPutchar (fd, writebuffer[i]);
		}
		return writelength;
	}

	inline int8_t sendAddressChecksum(uint8_t a, uint32_t d, int &fd){
		writeChecksum(a,d);
		for(int i=0;i<writelength;i++){
			if(fd<0) return -1;
			serialPutchar (fd, writebuffer[i]);
		}
		return writelength;
	}

	inline int8_t sendCommand(uint8_t c, int &fd){
		if(fd<0) return -1;
		write(c);
		serialPutchar (fd, writebuffer[0]);
		return writelength;
	}

#else //Arduino
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
		write(a,d);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

	inline uint8_t sendAddressChecksum(uint8_t a, uint32_t d, Stream &s){ return sendAddressChecksum(a,d,&s);}
	inline uint8_t sendAddressChecksum(uint8_t a, uint32_t d, Stream *pStream){
		writeChecksum(a,d);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

	inline uint8_t sendCommand(uint8_t c,Stream &s){ return sendCommand(c,&s);}
	inline uint8_t sendCommand(uint8_t c, Stream *pStream){
		write(c);
		pStream->write(writebuffer, writelength);
		return writelength;
	}

#endif
};

extern NHProtocol NHP;
extern NHProtocol Protocol;

#endif
