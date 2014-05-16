#how to compile:
#cd /home/pi/Desktop/Arduino/libraries/SerialProtocol/examples/Arduino_Serial
#sudo make && sudo ./Pi_Serial

CC=gcc
CFLAGS=-lwiringPi -DRaspberryPi -c -pedantic -Wall
LDFLAGS=-lwiringPi -DRaspberryPi -pedantic -Wall
SOURCES=Pi_Serial.cpp ../../SerialProtocol.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Pi_Serial

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	rm  *.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm  *.o $(EXECUTABLE)

rebuild: clean all