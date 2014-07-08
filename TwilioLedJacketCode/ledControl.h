///////////////////////////////////////////////////////////////////
// A set of functions, types and buffers used for LED control.
// Most important is the 'colorBuffer' Array, which contains the colors that will be sent to the LEDs
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"
#include "SPI.h"
//we'll use this to define colors
struct Color{
  byte r;
  byte g;
  byte b;
};


//here we store the LED colors
Color colorBuffer[nTotalLeds]; 

int brightnessMultiplier=0; // this is initialized in 'setupLedControl' to a safe value for current limitig.

// To allow for a distinction between two different Groupts of LEDs, we have to know the postions of the smaller group:
#define nLogoLeds 4
#define nDisplayLeds nTotalLeds-nLogoLeds
int logoLeds[]={
  0,1,2,3}; // the positions of the Twilio Logo LEDs in the chain

int displayLeds[nTotalLeds-nLogoLeds]; // will hold all other positions - initialized in the LedEffectEngine constructor

// a convencience function to check if an array contains a value
bool contains(int* array, int length,int value){
  for(int i=0;i<length;i++){
    if(array[i]==value) return true;
  }
  return false;
};

void setupDisplayLedIndices(){
  // create a list that only contains the LEDs that shall be used for display
  int otherLedCount=0;
  for(int i=0;i<nTotalLeds;i++){
    if(!contains(logoLeds,nLogoLeds,i)){
      displayLeds[otherLedCount]=i;
      otherLedCount++;
    }
  }
}

// initialized SPI to talk to the LEDs
void setupLedControl(){
  //we use SPI to send data to the LEDs:
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  // SPI.setClockDivider(SPI_CLOCK_DIV16); // 1 MHz
  SPI.setClockDivider(SPI_CLOCK_DIV8); // 2 MHz
  // SPI.setClockDivider(SPI_CLOCK_DIV4); // 4 MHz
  //calculate a brightness scaling factor that honors the current limit:
  float totalMaxCurrent=nTotalLeds*3*20; //each color draws 20mA max per Pixel
  float safeMultiplier=totalCurrentLimit/totalMaxCurrent;
  brightnessMultiplier=floor(safeMultiplier*255);
  // we use bitshifting to modify the brightness values because it is faster than dividing
  // but this means we can only 'divide' by powers of two.
  //&brightnessBitShift= (int)ceil(log(1.0/safeMultiplier)/log(2.0)); // this calculates the bitshift that is always on the safe side.
  TRACE(F("brightness multiplier:"));
  TRACELN(brightnessMultiplier); 
};



//this function sends the data stored in colorBuffer to the LEDs
void sendColorBuffer(){
  byte* rawData = (byte*)colorBuffer; // we treat the color array as raw bytes to speed up the next step.
  int dataLength=nTotalLeds*3;
  for (int i=0;i<dataLength;i++){
    unsigned int limitedColorPixelData=((unsigned int)rawData[i]*brightnessMultiplier)>>8;
    //byte limitedColorPixelData=rawData[i]>>brightnessBitShift; // reduce the brightness value to limit power consumption
    //load the data into the SPI DATA register (SPDR) and wait for the transmission to be finished (SPSR & _BV(SPIF)).
    for (SPDR = (byte)limitedColorPixelData; !(SPSR & _BV(SPIF)););
  }
};
