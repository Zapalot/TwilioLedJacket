///////////////////////////////////////////////////////////////////
//some convenience functions for receiving data from streams
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once

//throw away all data that is left in the receive buffer
void flushRxData(Stream* source){
  while(source->available()){
    source->read();
  };
}

//read Data until the buffer is full or nothing comes any longer...
int receiveDataFromStream(Stream* source, char* buffer, int bufLength, long initialTimeout,long interCharTimeout){
  long startTime=millis();
  int bytesReceived=0;
  while(!source->available()&&millis()-startTime<initialTimeout){
  } // first byte to arrive
  if(!source->available())return -1; // got nothing...
  //read until buffer full or timeout
  startTime=millis(); //reset timout
  while(source->available()||((millis()-startTime)<interCharTimeout)){
    int inByte=source->read(); //read even if buffer is already full - then we just throw away everything that follows
    //if we got data...
    if(inByte>0){
      startTime=millis();
      //store incoming data if there is space left
      if(bytesReceived<bufLength-1){
        buffer[bytesReceived]=inByte;
        bytesReceived++;
      }
    }

  }
  buffer[bytesReceived]=0; //add zero termination to incoming data string.
  return(bytesReceived);
}
