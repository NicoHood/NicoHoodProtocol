
// Local-Hyperion includes
#include "LedDeviceTest.h"

LedDeviceTest::LedDeviceTest(const std::string& output) :
	_ofs(output.empty()?"/home/pi/LedDevice.out":output.c_str())
{
	// empty
}

LedDeviceTest::~LedDeviceTest()
{
	// empty
}

int LedDeviceTest::write(const std::vector<ColorRgb> & ledValues)
{
	_ofs << "[";
	for (const ColorRgb& color : ledValues)
	{
		_ofs << color;
	}
	_ofs << "]" << std::endl;

	return 0;
}

int LedDeviceTest::switchOff()
{
	return 0;
}



// NicoHood

LedDeviceMemory::LedDeviceMemory(int sharedmemorykey){
	/// the number of leds
	_ledCount=0;

	// Key to communicate between the two programs default: 1213
	_shKey=sharedmemorykey;

	// create shared memory, 3byte for each led + 1 led size byte
	_shID = shmget(_shKey, SH_LEDSIZE*3+1, IPC_CREAT | 0666);
	if (_shID == -1) {
		// error creating the share
		std::cerr << "Could not create shared memory with key: " << _shKey << std::endl;
		exit(1); //change? <--
	}
	else{
		// attach memory
		_shMemory = (uint8_t *)shmat(_shID, 0, 0);
		if (_shMemory==(uint8_t *)-1) {
			std::cerr << "Could not attach memory" << std::endl;
			exit(1); //change? <--
		} 
		else{
			// set the led length to the first byte
			_shMemory[0]=_ledCount;
			std::cout << "Successfully created shared memory with key: " << _shKey << std::endl;
			// detach memory
			shmdt(_shMemory);
		}
	}
}

LedDeviceMemory::~LedDeviceMemory()
{
	// delete memory
	shmctl(_shID, IPC_RMID, 0);
	std::cout << "Successfully detached and deleted shared memory" << std::endl;
}


int LedDeviceMemory::write(const std::vector<ColorRgb> & ledValues)
{
	// attach memory
	_shMemory = (uint8_t *)shmat(_shID, 0, 0);
	if (_shMemory==(uint8_t *)-1) {
		std::cerr << "Could not attach memory" << std::endl;
		return-1;
	} 

	// check maximum led size
	if (ledValues.size() > SH_LEDSIZE) {
		std::cerr << "Invalid attempt to write led values. Not more than " << SH_LEDSIZE << " leds are allowed." << std::endl;
		return -1;
	}

	// resize buffer if needed
	if (_ledCount != ledValues.size()) {
		std::cout << "Resizing Leds" << std::endl;
		// set the led length to the first byte
		_shMemory[0] = _ledCount = ledValues.size();
	}

	// write the leds to the shared memory
	for (unsigned i=0; i<ledValues.size(); i++)	{
		const ColorRgb& color = ledValues[i];
		// 1 offset for the length indicator
		_shMemory[1+i*3] = color.red;
		_shMemory[1+i*3+1] = color.green;
		_shMemory[1+i*3+2] = color.blue;
	}

	// debug output
	//std::cout << "Wrote " << _ledCount << " Leds to shared Memory" << std::endl;

	// detach memory
	shmdt(_shMemory);

	return 0;
}

int LedDeviceMemory::switchOff()
{
	// clear Leds
	write(std::vector<ColorRgb>(_ledCount, ColorRgb{0,0,0}));
	return 0;
}


// NicoHood

LedDeviceSerial::LedDeviceSerial()
{
	// clears the number of Leds and the updated Led
	_ledCount=0;
	_updateLed=0;

	// default turned off
	_status=false;
}

LedDeviceSerial::~LedDeviceSerial()
{
	// empty
}


int LedDeviceSerial::write(const std::vector<ColorRgb> & ledValues){
	// check Serial for new Commands
	// no serial error output here (only commands, not needed)
	while(Protocol.read()){
		if(Protocol.getCommand()==4){
			_status=false;
			std::cout << "Ambilight set to off" << std::endl;
		}
		else if(Protocol.getCommand()==3){
			_status=true;
			std::cout << "Ambilight set to on" << std::endl;
		}
		else if(Protocol.getCommand()==2){
			std::cout << "Pong!" << std::endl;
		}
		else if(Protocol.getCommand()==1){
			std::cout << "Ping!" << std::endl;
			// send Pong
			Protocol.sendCommand(2);
		}
		else {
			std::cerr << "Invalid Command via Serial." << std::endl;
		}
	}

	// return if Ambilight is turned off
	if(_status==false) return -1;

	// check maximum led size
	if (ledValues.size() > 255) {
		std::cerr << "Invalid attempt to write led values. Not more than " << 255 << " leds are allowed." << std::endl;
		return -1;
	}

	// resize buffer if needed
	if (_ledCount != ledValues.size())	{
		std::cout << "Resizing Leds" << std::endl;
		_ledBuffer.resize(ledValues.size() * 3);
		_ledCount = ledValues.size();
		_updateLed=0;
	}

	// write the leds to the Serial
	uint8_t * dataPtr = _ledBuffer.data();
	uint8_t updateCount=0;
	for (uint8_t i=0; i<ledValues.size(); i++)	{
		// temporary helper values to compare new leds with the old buffer leds
		const ColorRgb& color = ledValues[i];
		uint8_t red   = color.red;
		uint8_t green = color.green;
		uint8_t blue  = color.blue;

		uint8_t buffred   = _ledBuffer[i*3];
		uint8_t buffgreen = _ledBuffer[i*3+1];
		uint8_t buffblue  = _ledBuffer[i*3+2];

		// only write updated leds to the serial to speed up/ dont use too much serial buffer
		// problem is that the receiver might miss some constant, not updated data over the time
		// this might be a problem for sending everything black
		// therefore we update at least one led for every update.
		if(buffred!=red || buffgreen!=green || buffblue!=blue || i==_updateLed){
			uint32_t colors = (red<<(8*3)) + (green<<(8*2)) + (blue<<8) + i ;
			Protocol.sendAddress(1, colors);
			updateCount++;
		}
		// write previous color data to the buffer
		*dataPtr++ = color.red;
		*dataPtr++ = color.green;
		*dataPtr++ = color.blue;
	}
	// debug output
	//std::cout << "Updating " << int(updateCount) << " of " << ledValues.size() << " Leds" << std::endl;

	// next time update the next Led in the strip
	_updateLed++;
	if(_updateLed==_ledCount) _updateLed=0;

	// Write an Update Command to the Serial to update leds
	Protocol.sendCommand(5);
	return 0;
}

int LedDeviceSerial::switchOff(){
	if(_status==true){
		// clear Leds
		write(std::vector<ColorRgb>(_ledCount, ColorRgb{0,0,0}));

		// sends off request
		std::cout << "Sending Serial turn off request" << std::endl;
		Protocol.sendCommand(4);
		// return send output <--
		return 0;
	}
	else{
		std::cerr << "Serial already turned off" << std::endl;
		return 0;
	}
}


int LedDeviceSerial::open(const char serialDevice[], const unsigned long serialBaud){
	//get filedescriptor
	if ((_fd = serialOpen (serialDevice, serialBaud)) < 0){
		std::cerr << "Unable to open serial device" <<  serialDevice << " at baud " << serialBaud << std::endl;
		exit(1); //<-- change?
	}
	else{
		std::cout << "Successfully opened serial device" <<  serialDevice << " at baud " << serialBaud << std::endl;
	}

	// set Serial for Protocol and send a Ping
	Protocol.setSerial(_fd);

	// sends a Ping
	std::cout << "Sending Serial Ping" << std::endl;
	Protocol.sendCommand(1);

	// sends on request
	std::cout << "Sending Serial turn on request" << std::endl;
	Protocol.sendCommand(3);

	return _fd;
}