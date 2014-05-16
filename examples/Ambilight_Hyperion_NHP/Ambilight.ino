void checkAmbilight(void){
  // check ambilight timeout if program suddenly
  // exits without any off signal.
  if(stateAmbilight==true && millis()-prevAmbilight>=5000){
    //Serial.println("Ambilight Timeout");
    ledError();
    ambilightOff();
  }
}

void ambilightInput(uint32_t input){
  if(ambilightIsOff()) return;

  //4byte: red, green, blue, led number,
  uint8_t number = input&0xFF;
  uint32_t rgb =input>>8;

  // write data to the led array (ensure that there is no more data than we have in our strip)
  if(number<numLedsTV) ledsTV[number]= rgb;
  else ledError();
}

void ambilightUpdate(void){
  if(ambilightIsOff()) return;
  FastLED.show();
  prevAmbilight=millis();
}

boolean ambilightIsOff(void){
  if(stateAmbilight==false) {
    // we have turned ambilight off. this is an error or just the rest of the buffer
    //Serial.println("Already off");
    ledError();
    ambilightOff();
    //Protocol.sendCommand(commandAmbilightOff, Serial);
    return true;
  }
  return false;
}

void ambilightOff(void){
  //Serial.println("Ambilight off");
  ledStatus();
  FastLED.clear();
  FastLED.show();
  stateAmbilight=false;
  NHP.sendCommand(commandAmbilightOff, Serial);
}

void ambilightOn(void){
  if(stateAmbilight==true) {
    ledError();
    //Serial.println("Already on");
    return;
  }
  //Serial.println("Ambilight on!");
  ledStatus();
  stateAmbilight=true;
  ambilightUpdate();
  NHP.sendCommand(commandAmbilightOn, Serial);
}

