///////////////////////////////////////////////////////////////////
// this class is responsible for blinking the logo before a new effekt is played.
// whenever a new message is received, it is buffered until blinking is over and parsed only later.
// because we are short on memory already, we do not implement a message queue.
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
#include "debugTrace.h"
#include "commands.h"
#include "ledControl.h"

class ReceiveIndicatorHandler{
private:
  unsigned long totalBlinkTime;  ///< how long in total should the reveive Animation be played?
  unsigned long blinkInterval;   ///< how long should each LED-Flash take?
  unsigned long startMillis;
  bool incomingMessage;          ///< if true, the logo flashes to indicate incoming message
  char* receiveBuffer;           ///< a buffer where the incoming message is stored while the indicator leds blink
  CommandParser* messageParser;  ///< after blinking ist finished, the message is forwarded to this parser
public:
  ReceiveIndicatorHandler(char* receiveBuffer,  CommandParser* messageParser,long totalBlinkTime, long blinkInterval);
  void newMessageReceived();          ///< resets the blink timer
  void update();                      ///< Update the LED colors according to the selected effect
};


ReceiveIndicatorHandler::ReceiveIndicatorHandler(char* receiveBuffer,  CommandParser* messageParser,long totalBlinkTime, long blinkInterval){
  this->receiveBuffer=receiveBuffer;
  this->messageParser=messageParser;
  this->totalBlinkTime=totalBlinkTime;
  this->blinkInterval=blinkInterval;
  startMillis=0;
  incomingMessage=false;
};

void ReceiveIndicatorHandler::newMessageReceived(){
  startMillis=millis();
  incomingMessage=true; //enable receive signal
}


//Update the LED colors according to the selected effect
void ReceiveIndicatorHandler::update(){
  long totalBlinkTime=blinkInterval*5*7;
  //slowly fade in and out Twilio Logo LEDs
  //the white ones:
  for(int i=0;i<nLogoLeds;i+=2){
    colorBuffer[logoLeds[i]]=(Color){
      255,255,255      }; 
  }
  //the red ones:
  float fadeFrequency=0.5; //in Hz
  float fadeIntensity=(128.0+128.0*sin(((float)millis())/1000.0*fadeFrequency*PI));
  for(int i=1;i<nLogoLeds;i+=2){
    colorBuffer[logoLeds[i]]=(Color){
      fadeIntensity,0,0      };
  }

  if(incomingMessage){
    unsigned long millisSinceStart=millis()-startMillis;
    if(millisSinceStart>totalBlinkTime){
      //push new data to effect engine
      TRACELN(F("invoking message parser from receive handler"));
      messageParser->parseString(receiveBuffer,strlen(receiveBuffer));
      incomingMessage=false;
    }
    else{
      //indicate received message by strobing twilio logo leds
      bool ledsOn=!((millisSinceStart/blinkInterval)&1)&&!((millisSinceStart/(blinkInterval*7))&1); // light the leds in even intervals of the blinkTime
      TRACE(F("logo is:"));
      TRACELN(ledsOn);
      for(int i=1;i<nLogoLeds;i+=2){
        colorBuffer[logoLeds[i]]=ledsOn?(Color){
          255,0,0        }
        :(Color){
          0,0,0        };
      }


    }
  }
}

///this function automatically distributes incoming data from a stream to the parameter setting instances
void receiveMessagesFromStream(Stream* inStream,char* messageBuffer,int messageBufferLength,ReceiveIndicatorHandler* messageHandler){
  if(!inStream->available())return;  //if there is no data available, dont even start timeouts
  //read data from stream
  int bytesRead= inStream->readBytes(messageBuffer,messageBufferLength -1);
  messageBuffer[bytesRead]=0; //add zero termination to input buffer
  messageHandler->newMessageReceived();
};




