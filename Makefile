#how to compile:
#cd /home/pi/Desktop/Arduino/libraries/NicoHoodProtocol/
#sudo make install

install: NicoHoodProtocol.cpp
	gcc -Wall -fPIC -c NicoHoodProtocol.cpp -lwiringPi -DRaspberryPi -c -pedantic
	gcc -shared -Wl,-soname,libNicoHoodProtocol.so.1 -o libNicoHoodProtocol.so.1.0   *.o
	install -m 0644 NicoHoodProtocol.h /usr/local/include
	install -m 0755 -d /usr/local/lib
	install -m 0755 libNicoHoodProtocol.so.1.0 /usr/local/lib/libNicoHoodProtocol.so.1.0
	ln -sf /usr/local/lib/libNicoHoodProtocol.so.1.0 /usr/lib/libNicoHoodProtocol.so
	ldconfig
	rm *.o *.so.1.0
	@echo Successfully installed NicoHoodProtocol library

hyperionmod:
	set -e sudo killall hyperiond
	sudo cp ./HyperionSource/hyperion-remote /usr/bin/
	sudo cp ./HyperionSource/hyperiond /usr/bin/
	@echo Successfully installed hyperionmod
	@echo please restart hyperion or reboot now

hyperionconfig:
	sudo cp ./HyperionSource/hyperion.config.json /etc/hyperion.config.json
	@echo Successfully installed hyperionconfig

hyperioncopysource:
	sudo cp ./HyperionSource/LedDevice* ./hyperion/libsrc/leddevice/
	@echo Successfully copied sourcefiles

serialtest: examples/Arduino_Serial/Pi_Serial.cpp
	gcc -o examples/Arduino_Serial/Pi_Serial.o examples/Arduino_Serial/Pi_Serial.cpp -DRaspberryPi -lwiringPi -lNicoHoodProtocol -pedantic -Wall

ambilight: examples/Ambilight/Ambilight_Serial.cpp
	gcc -o examples/Ambilight/Ambilight_Serial.o examples/Ambilight/Ambilight_Serial.cpp -DRaspberryPi -lwiringPi -lNicoHoodProtocol -pedantic -Wall

