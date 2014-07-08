///////////////////////////////////////////////////////////////////
// some useful functions and structures for calculation in 2D-space
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
//this one is mainly used for storage
struct CharVec2d
{
  char x;
  char y;
};

//this one is handy for calculations
struct FloatVec2d{
  float x;
  float y;
};

//a lot of graphical effects are based on distance, so we declare a convenience function for that
inline float distance(float x1, float y1, float x2, float y2){
  return sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

// we will use distances between leds a lot...
inline float distance(CharVec2d p1, CharVec2d p2){
  return(distance(p1.x,p1.y,p2.x,p2.y));
}
//same for floating point vectors
inline float distance(FloatVec2d p1, FloatVec2d p2){
  return(distance(p1.x,p1.y,p2.x,p2.y));
}

// calulate length of a vector
inline float length(FloatVec2d p){
return sqrt(p.x*p.x+p.y*p.y);
}
//return a vector of length 1
inline FloatVec2d normalize(FloatVec2d p){
  float len=length(p);
  return((FloatVec2d){p.x/len, p.y/len});
}

