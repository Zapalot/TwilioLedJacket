///////////////////////////////////////////////////////////////////
// Generative Effects for LEDs
// Acts on the 'colorBuffer' defined in 'ledControl.h'
// Uses LED-Positions defined in 'ledPosition.h'
///////////////////////////////////////////////////////////////////

#pragma once
#include "vec2d.h"        // some useful functions and structures for calculation in 2D-space
#define NUM sizeof(positions)/sizeof(Point)

//positions of the leds as from the editor
PROGMEM CharVec2d ledPositions[34] =
{
  -10,-4,
  -10,-3,
  -9,-3,
  -9,-4,
  -4,-3,
  -4,-2,
  -4,-1,
  -5,-2,
  -5,-1,
  -5,0,
  -5,1,
  -5,2,
  -5,3,
  -4,2,
  -3,1,
  -3,2,
  -3,3,
  -3,4,
  -3,5,
  3,5,
  3,4,
  3,3,
  3,2,
  3,1,
  4,2,
  5,3,
  5,2,
  5,1,
  5,0,
  5,-1,
  5,-2,
  4,-1,
  4,-2,
  4,-3,
};

// easy acces to an LED Position
CharVec2d getLedPosChar(int index){
    CharVec2d ledPosChar;
    ledPosChar.x=pgm_read_byte(&(ledPositions[index].x));
    ledPosChar.y=pgm_read_byte(&(ledPositions[index].y));
    return(ledPosChar);
};

FloatVec2d getLedPosFloat(int index){
    FloatVec2d ledPosFloat;
    ledPosFloat.x=(float)((char)pgm_read_byte(&(ledPositions[index].x)));
    ledPosFloat.y=(float)((char)pgm_read_byte(&(ledPositions[index].y)));
    return(ledPosFloat);
}

