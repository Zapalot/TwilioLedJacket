///////////////////////////////////////////////////////////////////
// Generative Effects for LEDs
// Acts on the 'colorBuffer' defined in 'ledControl.h'
// Uses LED-Positions defined in 'ledPosition.h'
//
// Rainbowfade Effect by Stefan Hintz/TrafoPop.
//
// The rest of the code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
#include "vec2d.h"        // some useful functions and structures for calculation in 2D-space
#include "ledControl.h"
#include "ledPositions.h"

///////////////////////////////////////////
/// this class implements our LED-Effects
class LedEffectEngine{
public:
  // each implemented Effect is one of the modes below:
  enum EffectMode{
    flash,     ///< random flashes of all leds at the same time - a bit like a thunderstorm
    glitter,   ///< single LEDs light up randomly and decay again
    fade,      ///< fading back and forth between colors - position dependent
    fill,      ///< a moving block of lit LEDs is followed by a dark one - might become position dependent for TrafoPop Jacket
    moveDots,  ///< single LEDs with primary color move in a secondary color background
    rainbow    ///< rainbow fade galore from original TrafoPop code
  };

  LedEffectEngine();

  // all kinds of parameters that determine what is shown
  Color primaryColor;        ///< first brush color
  Color secondaryColor;      ///< second brush color
  long lastUpdateMillis;     ///< time of the last update
  long lastModeChangeMillis; ///< time of the last effect change
  
  //control of colors:
  void setEffect(EffectMode newEffect); ///< set the effect that is currently being played
  EffectMode getEffect();               ///< get currently selected Effect
  void updateColors();                  ///< update the LED colors according to the selected effect
  void doConnectionBad();               ///< show a pattern that indicates a lost GSM connection
  
  //these functions implement the effects we have... 
  void doFlash();
  void doGlitter();
  void doFade();
  void doFill();
  void doMoveDots();
  void doRainbow(); // this one shows how to use positions for animation!


private:
  EffectMode selectedEffect; ///< the effect that is currently being played
};

////////////////////////////////////////////
//Implementations

//initial setup of an instance
LedEffectEngine::LedEffectEngine(){

  //set inititial colors
  primaryColor=(Color){
    255,255,255            };
  secondaryColor=(Color){
    0,0,0            };
};

// select the effect to be played
void LedEffectEngine::setEffect(EffectMode newEffect){
  selectedEffect=newEffect;
  lastModeChangeMillis=millis();
}
//get currently selected Effect
LedEffectEngine::EffectMode LedEffectEngine::getEffect(){
  return selectedEffect;
};               

//Update the LED colors according to the selected effect
void LedEffectEngine::updateColors(){
  unsigned long startMillis=millis(); // keeping time this way makes it easier for effects to work independently of framerate.
  switch(selectedEffect){
  case flash:
    doFlash();
    break;
  case fade:
    doFade();
    break;
  case fill:
    doFill();
    break;
  case glitter:
    doGlitter();
    break;
  case moveDots:
    doMoveDots();
    break;
  case rainbow:
    doRainbow();
  };
  lastUpdateMillis=startMillis;
}

// light up all LEDs in random intervals
void LedEffectEngine::doFlash(){
  //a 1:10 chance to switch all the LEDs to the primary Color...
  Color ledColor=random(1000)<100?primaryColor:(Color){
    0,0,0            };
  //TRACELN(ledColor.r);
  for(int i=0;i<nDisplayLeds;i++){
    colorBuffer[displayLeds[i]]=ledColor;
  }
  delay(20); // to keep the flash a while
};

// fade Leds back and forth between colors
void LedEffectEngine::doFade(){
  //a 1:100 chance to switch all the LEDs to the primary Color...
  const float fadeTime=1500;
  unsigned long timeSinceSet= millis()-lastModeChangeMillis;
  unsigned long timeSinceUpdate= millis()-lastUpdateMillis;
  //first, we do an exponential fade towards the primary color:

    //set all the leds to that color
    for(int i=0;i<nDisplayLeds;i++){
      // do a sinus modulated fade between primary and secondary color
      float rotationfreq=0.00015; //per millisecond
      float xScale=0.5*sin((float)timeSinceSet*rotationfreq*2*PI);
      float yScale=0.6*cos((float)timeSinceSet*rotationfreq*2*PI);

      FloatVec2d ledPos=getLedPosFloat(displayLeds[i]); //
      float offset=ledPos.x*xScale+ledPos.y*yScale;

      float primaryFraction= 0.5+0.5*cos((float)timeSinceSet/(float)fadeTime-3.0+offset); // will be 1 at fade time
      Color fadeColor;

      //TRACE(F("Fading with fraction:"));
      //TRACELN(primaryFraction);
      fadeColor.r=(byte)((float)primaryColor.r*primaryFraction+(float)secondaryColor.r*(1.0-primaryFraction));
      fadeColor.g=(byte)((float)primaryColor.g*primaryFraction+(float)secondaryColor.g*(1.0-primaryFraction));
      fadeColor.b=(byte)((float)primaryColor.b*primaryFraction+(float)secondaryColor.b*(1.0-primaryFraction));
      colorBuffer[displayLeds[i]]=fadeColor;
    }
  
}

// each led has a small random chance to light up and will then fade out
void LedEffectEngine::doGlitter(){
  unsigned long timeSinceUpdate= millis()-lastUpdateMillis;
  float chancePerSecond=0.9;
  float chanceNow=chancePerSecond*(float)timeSinceUpdate/1000.0;
  int decay=15;
  long activationThresh=(long)(chanceNow*10000.0); // this will be compared to a random value
  //set all the leds to that color
  for(int i=0;i<nDisplayLeds;i++){
    bool isActive=random(10000)<activationThresh;
    if(isActive){
      colorBuffer[displayLeds[i]]=primaryColor;
    }
    else{
      colorBuffer[displayLeds[i]].r=constrain(colorBuffer[displayLeds[i]].r-decay,0,255);
      colorBuffer[displayLeds[i]].g=constrain(colorBuffer[displayLeds[i]].g-decay,0,255);
      colorBuffer[displayLeds[i]].b=constrain(colorBuffer[displayLeds[i]].b-decay,0,255);
    }
  }
  delay(1); // to slow down the fadeout
};

//Fill the LED chain one by one
void LedEffectEngine::doFill(){
  float fillSpeed=0.03; //LEDs per millisecond
  unsigned long timeSinceStart= millis()-lastModeChangeMillis;
  unsigned long shift=timeSinceStart*fillSpeed; // how many pixels has it moved?
  for(int i=0;i<nDisplayLeds;i++){
    bool ledOn=((shift+i)/(nDisplayLeds*2))&1; // a block of nDisplayLeds bright LEDs moves through the chain, folowed by nDisplayLeds dark ones
    colorBuffer[displayLeds[i]]=ledOn?primaryColor:(Color){
      0,0,0                        };
  }
  delay(1);
};

// shift colors glowing dots through the led chain
  // the first half of the leds moves forward, the other half backwards

void LedEffectEngine::doMoveDots(){
  float moveSpeed= 0.01; //LEDs per millisecond
  int gapLength=5; //how many LEDs are dark between the bright ones?
  
  //how much of the old color should be left per round?
  float fadeFraction=0.8;
  float oldFraction=fadeFraction;
  float newFraction=1.0- fadeFraction;
      
  unsigned long timeSinceStart= millis()-lastModeChangeMillis;

  //
  long offset=((long)((float)timeSinceStart*moveSpeed));

  for(int i=0;i<nDisplayLeds;i++){
    long pos=(i<15)?(i+offset):(i-offset);
    // for some
    if(i<15){
      pos=(i+offset);
    }else{
       pos=(i-offset);
    };
     //set the color of every gapLength' LED to primaryColor, move them by adding the time dependent 'pos' to the index
    if((pos)%(gapLength+1)==0){
      colorBuffer[displayLeds[i]]=primaryColor;
    }
    else{
      //fade out all other LEDs towards the secondary color
      colorBuffer[displayLeds[i]].r=colorBuffer[displayLeds[i]].r*oldFraction+secondaryColor.r*newFraction;
      colorBuffer[displayLeds[i]].g=colorBuffer[displayLeds[i]].g*oldFraction+secondaryColor.g*newFraction;
      colorBuffer[displayLeds[i]].b=colorBuffer[displayLeds[i]].b*oldFraction+secondaryColor.b*newFraction; 
    }
  }
  delay(1);
};

// the rainbow fade effect from the original TrafoPop code
void LedEffectEngine::doRainbow(){
  //TRACELN(F("Rainbow fun!"));
  //what follows is big bad uncommented voodoo from the original trafopop code - don't ask me what all this stuff does...
  // \begin vodoo
  float time= (float)millis()/1000.0/30;
  float s = 0.01 * (0.7 + 0.2 * sin(time * 0.000827/0.002));
  float r = 2.0 * M_PI * sin(time * 0.000742/0.002);
  float sinr = sin(r);
  float cosr = cos(r);

  FloatVec2d center1 = (FloatVec2d){
    cos(time), cos(time*0.535)    };
  FloatVec2d center2 = (FloatVec2d){
    cos(time*0.259), cos(time*0.605)    };
  // \end vodoo

  for(int i=0;i<nDisplayLeds;i++){
    //get the position of the LED on the jacket
    FloatVec2d ledPosFloat=getLedPosFloat(displayLeds[i]);
    // \begin voodoo 
    // this is NOT the Position of the LED, but something that depends on it..
    FloatVec2d position=(FloatVec2d){
      (s*ledPosFloat.x*cosr - s*ledPosFloat.y*sinr),
      (s*ledPosFloat.x*sinr + s*ledPosFloat.y*cosr)        };

    int size = 64;
    float d = distance(position, center1)*size;
    FloatVec2d color = (FloatVec2d){
      cos(d),
      sin(d)    
      };

      d = distance(position, center2)*size;
    color.x += cos(d);
    color.y += sin(d);

    float c = length(color)*0.25;

    FloatVec2d ncolor = normalize(color);
    float red = ncolor.x;
    float green = ncolor.y;
    float blue = c * (ncolor.x-ncolor.y);

    Color finalColor={
      max(0,red * 255),
      max(0,green * 255),
      max(0,blue * 255)
      };

      // \end voodoo

      colorBuffer[displayLeds[i]]=finalColor; // put the calculated Color into the LED color Array
  }
}

void LedEffectEngine::doConnectionBad(){
  for(int i=0;i<nDisplayLeds;i++){
    colorBuffer[displayLeds[i]].r=255;
    colorBuffer[displayLeds[i]].g=0;
    colorBuffer[displayLeds[i]].b=0;
  }
}





