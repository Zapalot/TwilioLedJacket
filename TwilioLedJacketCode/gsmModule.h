///////////////////////////////////////////////////////////////////
// A small GSM-Module interface for receiving SMS
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"
#include "debugTrace.h"
#include "streamFun.h"


class MimimalGSM{
public:
  /// set all information necessary to use the module and initialize it
  void setup(
  Stream* gsmSerial,  ///< serial Object the module is connected to (i.e. &Serial1 on a Leonardo , or some software Serial)
  int powerPin,       ///< pin connected to the Power on pin of the Module
  char* pinNumber,    ///< PIN-Number entered into the SIM-Card to unlock it
  char* buffer,       ///< a buffer for incoming data - has to be at least 32 bytes long
  int bufLength       ///< the length of the receive buffer
  );

  void initializeModule(char* simPin);     ///< switches on the module and sets parameters

  //high level functions 
  bool checkConnection();                 ///< returns true if GSM connection is established
  void enterSimPinIfNeeded(char* simPin); ///< enters SIM PIN i.e. after a reset of the module
  
  ///receives new text messages, stores text in buffer and returns true if somehing new is there
  bool receiveSMS(
  bool deleteReceived,  ///< should the received message be deleted from the sim card?
  char*messageBuffer,   ///< the received message is copied to this buffer.
  int messageBufferLength  ///< capacity of the buffer
  ); 

  // low level command sending and receiving
  void receiveData(long initialTimeout=1000,long interCharTimeout=100); ///< receives data from GSM Module.
  void sendCmdAndReceive(char* command, long initialTimeout=1000,long interCharTimeout=100); ///< send a command and save reveived reply to the buffer.
  void sendCmdAndReceive(const __FlashStringHelper*  command,long initialTimeout=1000,long interCharTimeout=100); ///< send a command and save reveived reply to the buffer
  bool isReply(const __FlashStringHelper* checkString); ///< looks if the reiceive buffer contains a certain string

  void update();       ///< checks for sms notifications and gsm connectivity in a non-blocking way.
  bool smsWaiting;     ///< an sms notification was received. Is set by the 'update' method.
  bool connectionGood; ///< is the gsm connection available? Is set by the 'update' method.
private:
  int powerPin;         ///< arduino pin connecte dto the 'power on' switch pin of the GSM Module.
  Stream* gsmSerial;    ///< our data link to the GSM Module
  char* buffer;         ///< buffer for incoming data
  int bufLength;        ///< length buffer for incoming data

  unsigned long lastSmsPollingTime;       ///< when was the last time we asked the module for incoming messages? - used by the 'update' method
  unsigned long lastConnectionCheckTime;  ///< when was the last time we asked the module for connectivity info? - used by the 'update' method
  unsigned long lastReplyTime;            ///< when was the last time we received data from the moulde at all?
};



//receives data from GSM Module
void MimimalGSM::receiveData(long initialTimeout,long interCharTimeout){
  receiveDataFromStream(gsmSerial,buffer,bufLength,initialTimeout,interCharTimeout);
}


// send a command and save reveived reply to the buffer
void MimimalGSM::sendCmdAndReceive(char* command,long initialTimeout,long interCharTimeout){
  flushRxData(gsmSerial);
  gsmSerial->println(command);
  receiveData(initialTimeout,interCharTimeout);
}

// send a command and save reveived reply to the buffer
void MimimalGSM::sendCmdAndReceive(const __FlashStringHelper*  command,long initialTimeout,long interCharTimeout){
  flushRxData(gsmSerial);
  gsmSerial->println(command);
  receiveData(initialTimeout,interCharTimeout);
}

// checks if the receive buffer contains a certain string
bool MimimalGSM::isReply(const __FlashStringHelper* checkString){
  return(strstr_P(buffer,(const char PROGMEM *)checkString)!=NULL);
}

//enters SIM PIN i.e. after a reset of the module
void MimimalGSM::enterSimPinIfNeeded(char* simPin){
  //set Pin-Number if requested
  sendCmdAndReceive(F("AT+CPIN?"));
  if(isReply(F("SIM PIN"))&&simPin!=0){
    TRACELN(F("Module wants PIN number!"));
    if(simPin!=0){
      TRACELN(F("Entering PIN"));
      flushRxData(gsmSerial);
      gsmSerial->print(F("AT+CPIN="));
      gsmSerial->println(simPin);
      receiveData(1000,1000); //pin Checking takes a bit longer
      if(isReply(F("OK"))){
        TRACELN(F("PIN accepted!"));
      }
      else{
        TRACELN(buffer);
        TRACELN(F("!!!PIN not accepted - be careful not to lock yourself out by too many attempts!!!"));
      }
    }
    else{
      TRACELN(F("No PIN provided.."));
    }
  }
  else{
    TRACELN(F("PIN reply"));
    TRACELN(buffer);

  }
}

  /// set all information necessary to use the module and initialize it
void MimimalGSM::setup(Stream* gsmSerial,int powerPin,char* pinNumber, char* buffer,int bufLength){
  //initialize member variables:
  this->gsmSerial=gsmSerial;
  this->buffer=buffer;
  this->bufLength=bufLength;
  this->powerPin=powerPin;
  this->lastSmsPollingTime=millis();
  this->lastConnectionCheckTime=millis();
  smsWaiting=false;   // an sms notification was received
  connectionGood=false; // is the gsm connection available?

  initializeModule(pinNumber); //check connection and set all parameters as we like them
}

// start up the module
void MimimalGSM::initializeModule(char* simPin){
  //check if module is answering...
  delay (500); // to let all old replies be discarded
  TRACELN(F("Contacting GSM-Module with AT"));
  sendCmdAndReceive(F("AT"),1000,100);
  TRACELN(F("received..."));
  TRACELN(buffer);
  if(!isReply(F("OK"))){
    TRACELN(F("GSM-Module doesn't answer - trying to power on..."));
    pinMode(powerPin,OUTPUT);
    digitalWrite(powerPin, HIGH);
    delay(1200);
    digitalWrite(powerPin, LOW);
    delay(2000);

    //check again...
    TRACELN(F("trying again.."));
    sendCmdAndReceive(F("AT"));
    TRACELN(F("received..."));
    TRACELN(buffer);
    if(!isReply(F("OK"))){
      TRACELN(F("GSM-Module still doesn't answer OK - check Serial connection and baud rate!"));
      return;
    }
    else{
      TRACELN(F("..got an OK from the GSM Module"));
    }
  }
  TRACELN(F("setting factory defaults..."));
  // reset factory defaults
  sendCmdAndReceive(F("AT&F"));
  enterSimPinIfNeeded(simPin);
  TRACELN(F("setting sms text mode..."));
  // set SMS mode to plain text
  sendCmdAndReceive(F("AT+CMGF=1"));

  TRACELN(F("setting sms notification mode..."));
  //sendCmdAndReceive(F("AT+CNMI=0,0")); // switch off direct message forwarding. - without flow-control, we could miss messages when the arduino is busy.
  sendCmdAndReceive(F("AT+CNMI=2,1")); // get status messages when an sms is received. we use this to poll only when necessary.
  TRACELN(F("finished GSM-Module setup"));


}

// returns true if GSM connection is established
bool MimimalGSM::checkConnection(){
  TRACELN(F("checking connection..."));
  sendCmdAndReceive(F("AT+CREG?"),500,20); // ask if we are "registered" in the network
  TRACELN(F("reply is:"));
  TRACELN(buffer);
  connectionGood=isReply(F("+CREG: 0,1"))||isReply(F("+CREG: 0,5"));
  return(connectionGood);  

}

// checks for sms notifications gsm connection in a non-blocking way.
void MimimalGSM::update(){
  //TRACELN(F("GSM update"));
  // any data waiting for us?
  if(gsmSerial->available()){

    TRACELN(F("picking up data"));
    TRACELN(buffer);
    receiveData(100,20);
  }

  //is it an sms notification or a positive sms poll reply?
  if(isReply(F("+CMTI:"))||isReply(F("CMGL:"))){
    TRACE("positive sms poll/notication:");
    TRACELN(buffer);
    smsWaiting=true;
  }

  if(isReply(F("+CREG:"))){
    lastReplyTime=millis();
    connectionGood=(isReply(F("+CREG: 0,1"))||isReply(F("+CREG: 0,5"))); // anything else is "no connection"
    //TRACELN(connectionGood);
  }

  //check connection periodically
  if(millis()-lastConnectionCheckTime>2500){
    //TRACELN("initiating periodic gsm-connection check..");
    gsmSerial->println(F("AT+CREG?"));
    lastConnectionCheckTime=millis();
  }

  //check for unread sms periodically
  if(millis()-lastSmsPollingTime>500){
    //TRACELN("initiating periodic sms polling..");
    gsmSerial->println(F("AT+CMGL=\"REC UNREAD\",1"));
    lastSmsPollingTime=millis();
  }

  //if we haven't heard anything from the module for a long time, it's probably off or disconnected.
  if(millis()-lastReplyTime>15000){
    connectionGood=false;
  }
  //clear buffer..
  buffer[0]=0;
} 


//receives new text messages, stores text in buffer and returns true if somehing new is there
bool MimimalGSM::receiveSMS(bool deleteReceived, char*messageBuffer, int messageBufferLength){

  TRACELN(F("checking for sms messages!"));
  //get list of all unread sms, but don't mark them as read yet;
  //sendCmdAndReceive(F("AT+CMGL=\"REC UNREAD\",1")); // receive only unread messages
  delay(200); //wait for old polls to arrive so we can discard them
  sendCmdAndReceive(F("AT+CMGL=\"ALL\",1"),500,200); //receive all kinds of messages
  TRACELN(buffer);
  // see if there is a new message - they start with "\n+CMGL: "
  char* firstChar=strstr_P(buffer,(const char PROGMEM *)F("\n+CMGL: "));
  if(!firstChar){
    TRACELN(F("got no messages!"));
    return false;
  }

  TRACELN(F("parsing messages.."));
  //try to extract memory storage location of the message.
  firstChar = strchr(firstChar,':'); //look for a ':' starting from the place where we found the first "\n+CMGL: "
  // if we found something
  if(!firstChar){
    TRACELN(F("got bad data!"));
    return false;
  }

  //parse the storage location on the sim-card
  int storageSlot = atoi(firstChar+1);

  // now read only this specific message in order to mark it as 'Read'
  flushRxData(gsmSerial);
  gsmSerial->print(F("AT+CMGR="));
  gsmSerial->println(storageSlot);
  receiveData();

  //check if we got a valid reply for this one message
  firstChar=strstr_P(buffer,(const char PROGMEM *)F("\n+CMGR: "));
  if(!firstChar){
    TRACELN(F("got bad data!"));
    return false;
  }
  firstChar++; //skip line break in "\n+CMGR: "

  TRACELN(F("got a message!"));
  //now extract the message content by looking for the end of the meta information header
  firstChar=strchr(firstChar+1,'\n');
  if(!firstChar){
    TRACELN(F("got bad data!"));
    return false;
  }

  firstChar++; //skip the line break
  //determine length of message
  int messLength=strlen(firstChar)-8; //skip the trailing "\r\n\r\nOK\r\n" 

  //copy message to beginning of buffer
  memmove(messageBuffer,firstChar,min(messLength,messageBufferLength));
  messageBuffer[messLength]=0; //add zero termination
  TRACELN(F("copied message to buffer!"));

  //delete the message from sim card if requested
  if(deleteReceived){
    gsmSerial->print(F("AT+CMGD="));
    gsmSerial->println(storageSlot);
    receiveData();
  }
  return (true);
}
















