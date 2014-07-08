///////////////////////////////////////
// Allows to set the LEDs to predefined RGB values stored in a set of arrays
// Works, but is disabled for the 'small' version of the jacket
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////


#pragma once
#include "Arduino.h"
#include "images.h"
#include "commands.h"
#include "debugTrace.h"

#define nImages 1
prog_char imageName0[] PROGMEM = "#ger";


prog_char * const imageNames[] = {imageName0};
const prog_uchar* const images[]={image0};
class LedImageSelector:public AbstractCommandReceiver{
  public:
  LedImageSelector();
  void receiveCommand(char* inBuffer, int length); ///< looks for image names in the message
  void applyOnLeds();
  int selectedImageIndex;
  unsigned long lastSetTime;
};
LedImageSelector::LedImageSelector(){
  selectedImageIndex=-1; //no Image selected
  lastSetTime=millis();
}
void LedImageSelector::receiveCommand(char*inBuffer, int length){
     TRACELN(F("imageSelector looks at command..."));
     //TRACELN((const char PROGMEM *)imageName0);
     
 for (int i=0;i<nImages;i++){
     //TRACELN(imageNames[i]);
   if(strstr_P(inBuffer,(const char PROGMEM *)imageNames[i])!=0){
     TRACE(F("selected Image "));
     TRACELN(i);
     selectedImageIndex=i;
     lastSetTime=millis();
   }
 }
}
// you MUST have images that are at least as long as the leds...
void LedImageSelector::applyOnLeds(){
  if(millis()-lastSetTime>15000)selectedImageIndex=-1;
  if(selectedImageIndex<0)return; //-1 means nothing selected
 int arrayIndex=0;
 for (int i=0;i<nDisplayLeds;i++){
   colorBuffer[displayLeds[i]].r=pgm_read_byte(&images[selectedImageIndex][i*3]);
   colorBuffer[displayLeds[i]].g=pgm_read_byte(&images[selectedImageIndex][i*3+1]);
   colorBuffer[displayLeds[i]].b=pgm_read_byte(&images[selectedImageIndex][i*3+2]);
 }
 
}
