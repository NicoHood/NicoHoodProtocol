/*
 Arduino_Serial_Test.ino - SerialProtocol library - demo
 Copyright (c) 2014 NicoHood.  All right reserved.
 Program to test serial communication*/

// indicator led and button
int ledPin = 9;
int buttonPin=8;
unsigned long lastButton=0;

void setup()
{
  // Pin setup
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);

  // for Leonardo you can wait for the debug Serial
  //delay(1000);
  //while(!Serial); 
  Serial1.begin(9600);
  Serial1.println("Hello World!");
  Serial1.println("My name is Arduino Micro. Via Serial 1.");

  Serial.begin(9600);
  Serial.println("Hello World!");
  Serial.println("My name is Arduino Micro. Via Usb.");
}

void loop()
{
  // send a ping with a button press
  if(digitalRead(buttonPin)==LOW && millis()-lastButton>=200){
    Serial1.println("Ping!");
    // debounce
    lastButton=millis();
  }

  // print received signal to debug output
  if(Serial1.available()){
    digitalWrite(ledPin, HIGH);
    char newChar=Serial1.read();
    // print to debug Serial
    Serial.write(newChar);
  }
  digitalWrite(ledPin, LOW);
}
