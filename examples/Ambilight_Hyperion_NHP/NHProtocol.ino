bool checkProtocol(Stream &p){
  // read from the Serial
  bool newinput = NHP.read(p);

  //================================================================================
  //Check Data
  //================================================================================

  switch(NHP.getAddress()){
  case 0:// No Address
    break;

  case addressAmbilight:
    // writes led information to the strip if enabled
    ambilightInput(NHP.getData());
    break;

  default :
    ledError();
    break;
  }//end switch

  //================================================================================
  //Check Commands
  //================================================================================

  switch(NHP.getCommand()){
  case 0:// No Command
    break;

  case commandPing:
    ledStatus();
    NHP.sendCommand(commandPong, p);
    break;

  case commandPong:
    ledStatus();
    break;

    // shutdown commands are only needed if you want that a 3rd device can
    // send a shutdown request to this Arduino. This Arduino only forwards the command
  case commandShutdown:
    ledStatus();
    NHP.sendCommand(commandShutdown, Serial);
    break;

  case commandReboot:
    ledStatus();
    NHP.sendCommand(commandReboot, Serial);
    break;

  case commandAmbilightOn:
    ambilightOn();
    break;

  case commandAmbilightOff:
    ambilightOff();
    break;

  case commandAmbilightUpdate:
    ambilightUpdate();
    break;

  default:
    ledError();
    break;
  }//end switch
  
  
 return newinput;
}






