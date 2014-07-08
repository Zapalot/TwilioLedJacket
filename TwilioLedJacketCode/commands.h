///////////////////////////////////////////////////////////////////
// A general framework for text based commands on Arduino/compatible
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////


#pragma once
#include "Arduino.h"


// The maximum number of different command-receivers supported by the command parser:
#define CMD_MAX_NUMBER 32

///////////////////////////////////////////////////////////////////
/// This base class provides an interface for receiving commands.
/// Instances that implement this interface can be added to the list of receivers of the 'CommandParser' class
/// Use this interface if you want to build something that reacts to multiple different commands
///////////////////////////////////////////////////////////////////
class AbstractCommandReceiver{
public:
  virtual void receiveCommand(char*, int length)=0; ///< The command parser will use this interface pass commands to the receivers.
};

///////////////////////////////////////////////////////////////////
/// StringCommandReceiver checks if a command matches a string in Flash memory and calls a function if it does.
/// Extend it if you want something that reacts to just one command
///////////////////////////////////////////////////////////////////
class PStringCommandReceiver: 
public AbstractCommandReceiver{
public:
  // initializes the command. has to be called before anything works. We can't use a constructor for this because of the way progmem strings are implemented.
  virtual void setup(
  const __FlashStringHelper* cmdString	///< input is parsed for this command string
  );

  virtual void receiveCommand(char* input, int length); ///< checks command and calls 'acceptCommand' if it matches 'cmdString'

private:
  virtual void acceptCommand(char* input, int length)=0;  ///< overwrite this to implement your own behavior
  const __FlashStringHelper* cmdString; ///< serial input is parsed for this command string
  int cmdStringLength;                  ///< used for string comparisons
};

void PStringCommandReceiver::setup(const __FlashStringHelper* cmdString){
  this->cmdString=cmdString;
  this->cmdStringLength=strlen_P((const char PROGMEM *)cmdString); 
}

void PStringCommandReceiver::receiveCommand(char* input, int length){
  TRACE(F("Matching cmd "));
  TRACE(cmdString);
  TRACE(F("*"));
  TRACE(cmdStringLength);
  TRACE(F(":to:"));
  TRACE(input);
  TRACE((F("*")));
  TRACE(length);
  TRACE(F("-"));
  TRACE(strlen(input));


  //check if the command matches the input:
  if(length<cmdStringLength)return; // the input has to be at least as long as the command...
  TRACELN(F("..."));
  int foundPos=strncmp_P(input,(const char PROGMEM *)cmdString,cmdStringLength);
  if(foundPos==0){
    //check if we got any potential arguments
    if(length>cmdStringLength){
      if(input[cmdStringLength]!=' ')return; //the character following the command has to be a space.
      TRACE(F("matched with space: "));
      TRACELN((cmdString));
      acceptCommand(input+ cmdStringLength+1, length-cmdStringLength+1); // skip space and pass on everything that follows the commandto a derived class.
    }
    else{
      TRACE(F("matched: "));
      TRACELN((cmdString));
      acceptCommand(input+ cmdStringLength, length-cmdStringLength); // pass on everything that follows the commandto a derived class.
    }
  }
}

///////////////////////////////////////////////////////////////////
/// This Receiver will call an arbitrary function if the command matches.
/// It provides a convenient way of invoking actions.
/// All the LED control is based on instances of this class
///////////////////////////////////////////////////////////////////
class CallbackCommandReceiver: 
public PStringCommandReceiver{
public:
  // use this method to set the command string and function that will be called by the receiver.
  void setup(
  const __FlashStringHelper* cmdString,	///< input is parsed for this command string
  void (*callbackFunction)(char* input, int length) ///< this function will be called if the command matches
  );
private:
  virtual void acceptCommand(char* input, int length);
  void (*callbackFunction)(char* input, int length);   ///< points to the function to be called 
};
///implementations...
void CallbackCommandReceiver::setup(
const __FlashStringHelper* cmdString,	///< input is parsed for this command string
void (*callbackFunction)(char* input, int length) ///< this function will be called if the command matches
){
  this->callbackFunction=callbackFunction;
  PStringCommandReceiver::setup(cmdString); // let the base class take care of the rest...
};

void CallbackCommandReceiver::acceptCommand(char* input, int length){
  callbackFunction(input, length); // pass the data to the callback function.
};



///////////////////////////////////////////////////////////////////
/// The CommandParser provides a central message distribution hub.
/// It's receive method 'parseString' splits the incoming string into parts beginning with the command character "#"
/// Each part of the message is forwarded messages to all the AbstractCommandReceivers that are registered to it.
///////////////////////////////////////////////////////////////////
class CommandParser{
private:
  AbstractCommandReceiver* receiverList[CMD_MAX_NUMBER]; ///< incoming commands will be distributed to the entries of this array.
  int nCommandsInList;                                   ///< how many commands are there in the commandList?
public:
  CommandParser();
  void parseString(char* messageString, int length);      ///< parse a complete Message and pass Commands starting with '#' to the receivers in the list.
  void addReceiver(AbstractCommandReceiver* receiver);    ///< add an additional command-receiver to the internal list.

  /// conveniently create a callback receiver and add it to the internal list. Note that this will consume memory that will never be freed again!
  void addCallback(
  const __FlashStringHelper* cmdString,	                  ///< input is parsed for this command string
  void (*callbackFunction)(char* input, int length)       ///< this function will be called if the command matches
  ); 
};

CommandParser::CommandParser(){
  nCommandsInList=0;
};


///split a message into parts beginning with '#' and forward them to the receivers
void CommandParser::parseString(char* messageString, int length){
  char key[] = "#";  // each command has to begin with a #
  int  cmdPos=strcspn(messageString,key);  // points to the current potential command
  while (cmdPos <length-1)
  {
    TRACE(F("Found command at:"));
    TRACELN(cmdPos);
    int cmdLength=strcspn(messageString+cmdPos+1,key)+1;
    messageString[cmdPos+cmdLength]=0;//insert a zero termination for easier argument processing

    // let all the receivers have a look...
    for (int i=0;i<nCommandsInList;i++){
      receiverList[i]->receiveCommand(messageString+cmdPos, cmdLength);
    }
    messageString[cmdPos+cmdLength]='#';//revert the a zero termination
    cmdPos+=cmdLength;
  }
}

  /// conveniently create a callback receiver and add it to the internal list. Note that this will consume memory that will never be freed again!
void CommandParser::addCallback(const __FlashStringHelper* cmdString,void (*callbackFunction)(char* input, int length)){
  CallbackCommandReceiver* newCmd= new CallbackCommandReceiver(); //create the new receiver - it 
  newCmd->setup(cmdString,callbackFunction);
  addReceiver(newCmd); // add it to the internal list
};

void CommandParser::addReceiver(AbstractCommandReceiver* receiver){
  // check if the internal list is already full and add the receiver
  if(nCommandsInList<CMD_MAX_NUMBER){
    receiverList[nCommandsInList]=receiver;
    nCommandsInList++;
  }
  else{
    Serial.print(F("Max number of receiver instances exceeded, could not add new one"));
  }
}







