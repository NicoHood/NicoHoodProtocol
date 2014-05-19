/*
NicoHoodProtocol.cpp - NicoHoodProtocol library - implementation
Copyright (c) 2014 NicoHood.  All right reserved.
Daniel Garcia from the FASTLED library helped me with this code
*/

#include "NicoHoodProtocol.h"

// extern Protocol for easy use
NHProtocol NHP;
NHProtocol Protocol;

//================================================================================
//Read
//================================================================================

bool NHProtocol::read(uint8_t input){
	//reset if previous read was with an input/error
	if(mErrorLevel){
		// cancel any pending data reads if a reset was triggered
		if(mErrorLevel & NHP_INPUT_RESET){
			mBlocks=0;
			mWorkData=0;
		}
		// if previous read was a lead error keep this byte
		if(mErrorLevel&NHP_ERR_LEAD){
			readbuffer[0]=readbuffer[readlength];
			readlength=1;
		}
		else readlength=0;
	}

	// reset fully read data
	mCommand=0;
	mAddress=0;
	mData=0;
	mErrorLevel=0;

	//write input to the buffer
	readbuffer[readlength]=input;
	readlength++;

	// check the lead/end/data indicator
	switch(input&NHP_MASK_START){

	case(NHP_MASK_LEAD): 
		{
			// we were still reading!  Log an error
			if(mBlocks){
				mErrorLevel |= NHP_ERR_LEAD | NHP_ERR_READ;
				readlength--;
			}

			// read command indicator or block length
			mBlocks = (input & NHP_MASK_LENGTH)>>3;
			switch(mBlocks){
			case 0:
			case 1:
				// save 4 bit command
				mCommand=(input & NHP_MASK_COMMAND)+1;
				mBlocks = 0;
				mErrorLevel |= NHP_INPUT_COMMAND | NHP_INPUT_NEW;
				return true;
				break;
			case 7:
				// save block length + first 4 data bits (special case)
				mWorkData = input & NHP_MASK_DATA_4BIT;
				mBlocks -=2;
				break;
			default:
				// save block length + first 3 data bits
				mWorkData = input & NHP_MASK_DATA_3BIT;
				mBlocks--;
				break;
			}
		}
		break;

	case(NHP_MASK_END): 
		{
			if(mBlocks--==1){
				// save data + address
				mAddress=(input&0x3F)+1;
				mData=mWorkData;
				mErrorLevel |= NHP_INPUT_ADDRESS | NHP_INPUT_NEW;
				return true;
			}
			else{
				// log an error, not ready for an address byte, and reset data counters
				mErrorLevel |= NHP_ERR_DATA | NHP_ERR_READ;
				mBlocks=0;
			}
		}
		break;

		//case NHP_MASK_DATA1/2?? <--
	default: //NHP_MASK_DATA1/2
		{
			if(mBlocks--<2){
				// log an error, expecting an address or header byte
				mErrorLevel |= NHP_ERR_END | NHP_ERR_READ;
				mBlocks=0;
			}
			else{
				// get next 7 bits of data
				mWorkData<<=7;
				// dont need &NHP_MASK_DATA_7BIT because first bit is zero!
				mWorkData|=input; 
			}
		} 
		break;
	} // end switch

	// no new input
	return false; 
}

//================================================================================
//Write
//================================================================================

void NHProtocol::write(uint8_t command){
	// send lead mask 11 + length 00|0 or 00|1 including the last bit for the 4 bit command
	writebuffer[0] = NHP_MASK_LEAD | ((command-1) & NHP_MASK_COMMAND);
	writelength=1;
}

void NHProtocol::write(uint8_t address, uint32_t data){
	// start with the maximum size of blocks
	uint8_t blocks=7;

	// check for the first 7 bit block that doesnt fit into the first 3 bits
	while(blocks>2){
		uint8_t nextvalue=(data>>(7*(blocks-3)));
		if(nextvalue>NHP_MASK_DATA_3BIT){
			// special case for the MSB
			if(blocks==7) {
				writebuffer[0] = nextvalue;
				blocks--;
			}
			break;
		}
		else{
			// write the possible first 3 bits and check again after
			writebuffer[0] = nextvalue;
			blocks--;
		}
	}

	// write the rest of the data bits
	uint8_t datablocks=blocks-2;
	while(datablocks>0){
		writebuffer[datablocks] = data & NHP_MASK_DATA_7BIT;
		data>>=7;
		datablocks--;
	}

	// write lead + length mask
	writebuffer[0] |= NHP_MASK_LEAD | (blocks <<3);

	// write end mask
	writebuffer[blocks-1] = NHP_MASK_END | ((address-1) & NHP_MASK_ADDRESS);

	// save the length
	writelength=blocks;
}

//================================================================================
//Old Verison of Send Command/Address
//================================================================================


//void NHProtocol::sendAddress(uint8_t address, uint32_t data){
//	// block buffer for sending
//	uint8_t b[6];
//
//	// b[5] has the ‘address’ byte
//	b[5] = 0x80 | ((address-1) & 0x3F);
//
//	// fill in the rest of the data, b[0]-b[4] is going to have data
//	// in MSB order, e.g. b[4] will have the lowest 7 bits, b[3] the next
//	// lowest, etc...
//	b[4] = data & 0x7F;
//
//	uint8_t blocks=2;
//
//	// only loop/shift if there's data to put out
//	while(data>0x7F) {
//		data >>= 7;
//		b[6-(++blocks)] = data & 0x7F;
//	}
//
//	// if we can fit our highest bits in the first block, add them here
//	if((blocks==6) || b[6-blocks] < 8) {
//		// add to existing data
//	}
//	// if not just initialize the next byte
//	else b[6-(++blocks)] =  0;
//
//	// add in the block count to lead
//	b[6-blocks] |=  0xC0 | (blocks<<3);
//
//	// now write out the data - the blocks array in reverse, which will
//	// get your data written out in LSB order
//#ifdef RaspberryPi
//	for(int i=0;i<blocks;i++){
//		serialPutchar (mSerial, b[6-blocks+i]);
//	}
//#else //Arduino
//	// we need to write the buffer as array to work for the Wire library
//	mSerial->write(&b[6-blocks],blocks);
//#endif
//
//	// return the number of blocks written to the Serial
//	return blocks;
//}


//void NHProtocol::sendAddress(uint8_t address, uint32_t data){
//	// block buffer for sending
//	uint8_t b[6];
//
//	// b[0] has the ‘address’ byte
//	b[0] = 0x80 | ((address-1) & 0x3F);
//
//	// fill in the rest of the data, b[1]-b[5] is going to have data
//	// in LSB order, e.g. b[1] will have the lowest 7 bits, b[2] the next
//	// lowest, etc...
//	uint8_t blocks=0;
//	b[++blocks] = data & 0x7F;
//
//	// only loop/shift if there's data to put out
//	while(data > 0x7F) {
//		data >>= 7;
//		b[++blocks] = data & 0x7F;
//	}
//
//	// setup the header
//	uint8_t lead = 0xC0;
//
//	// if we can fit our highest bits in the first block, add them here
//	if((blocks==5) || b[blocks] < 8) {
//		lead |= b[blocks--] & 0x0F;
//	}
//
//	// add in the block count to lead
//	lead |= (blocks+2)<<3;
//
//	// now write out the data - lead, then the blocks array in reverse, which will
//	// get your data written out in MSB order, ending with the address block
//	serWrite(lead);
//	do { 
//		serWrite(b[blocks]); 
//	} 
//	while(blocks--);
//}
