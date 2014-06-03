#pragma once

// STL includes0
#include <fstream>

// Leddevice includes
#include <leddevice/LedDevice.h>

///
/// Implementation of the LedDevice that write the led-colors to an
/// ASCII-textfile('/home/pi/LedDevice.out')
///
class LedDeviceTest : public LedDevice
{
public:
	///
	/// Constructs the test-device, which opens an output stream to the file
	///
	LedDeviceTest(const std::string& output);

	///
	/// Destructor of this test-device
	///
	virtual ~LedDeviceTest();

	///
	/// Writes the given led-color values to the output stream
	///
	/// @param ledValues The color-value per led
	///
	/// @return Zero on success else negative
	///
	virtual int write(const std::vector<ColorRgb> & ledValues);

	/// Switch the leds off
	virtual int switchOff();

private:
	/// The outputstream
	std::ofstream _ofs;
};

// NicoHood
#include <sys/ipc.h>
#include <sys/shm.h>

// maximum supported led size (255 or less)
#define SH_LEDSIZE 255

///
/// Implementation of the LedDevice that write the led-colors to
/// the shared memory
///
class LedDeviceMemory : public LedDevice
{
public:
	/// the number of leds
	size_t _ledCount;

	// contructor
	LedDeviceMemory(int sharedmemorykey);

	// deconstructor
	virtual ~LedDeviceMemory();

	/// Writes the given led-color values to the shared memory
	virtual int write(const std::vector<ColorRgb> & ledValues);

	/// Switch the leds off
	virtual int switchOff();

private:
	// ID for the shared memory
	int _shID;

	// pointer to the shared bytes
	uint8_t *_shMemory;

	// Key to communicate between the two programs
	int _shKey;
};


// Serial inclues
#include <wiringPi.h>
#include <SerialProtocol.h>


///
/// Implementation of the LedDevice that write the led-colors to
/// the Serial device via Protocol
///
class LedDeviceSerial : public LedDevice
{
public:
	// contructor
	LedDeviceSerial();

	// deconstructor
	virtual ~LedDeviceSerial();

	// opens Serial
	int open(const char serialDevice[], const unsigned long serialBaud);

	/// Writes the given led-color values to the Serial via Protocol
	virtual int write(const std::vector<ColorRgb> & ledValues);

	/// Switch the leds off
	virtual int switchOff();

private:
	/// Serial filedescriptor
	int _fd;

	/// the number of leds
	size_t _ledCount;

	/// Buffer for writing/written led data
	std::vector<uint8_t> _ledBuffer;

	// stores the led to update every cycle
	uint8_t _updateLed;

	// turnes Serial output on/off
	bool _status;
};
