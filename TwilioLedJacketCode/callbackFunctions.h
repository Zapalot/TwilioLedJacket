///////////////////////////////////////////////////////////////////
// All the functions that are invoked by text commands are located here
//
// This code is under A Creative Commons Attribution/Share-Alike License
// http://creativecommons.org/licenses/by-sa/4.0/
// 2014, Felix Bonowski
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"
#include "ledEffects.h"
#include "debugTrace.h"


//Effect Endinge Mode set commands
// this function is invoked by the "#flash" command
void startFlash(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::flash);
  TRACELN(F("start flash"));
};

// this function is invoked by the "#fade" command
void startFade(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::fade);
  TRACELN(F("start fade"));
};

// this function is invoked by the "#glitter" command
void startGlitter(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::glitter);
  TRACELN(F("start glitter"));
};
// this function is invoked by the "#fill" command
void startFill(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::fill);
  TRACELN(F("start fill"));
};

// this function is invoked by the "#move" command
void startMove(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::moveDots);
  TRACELN(F("start move"));
};

// this function is invoked by the "#rainbow" command
void startRainbow(char* mess,int len){
  effectEngine.setEffect(LedEffectEngine::rainbow);
  TRACELN(F("start rainbow"));
};

/////////////Colors
// this function is invoked by the "#green" command
void startGreen(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    0,255,0  };
  TRACELN(F("set to green"));
}

// this function is invoked by the "#cyan" command
void startCyan(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    0,255,255  };
  TRACELN(F("set to cyan"));
}

// this function is invoked by the "#blue" command
void startBlue(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    0,0,255  };
  TRACELN(F("set to blue"));
}

// this function is invoked by the "#mauve" command
void startMauve(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    224,176,255  };
  TRACELN(F("set to mauve"));
}

// this function is invoked by the "#pink" command
void startPink(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    255,0,255  };
  TRACELN(F("set to pink"));
}

// this function is invoked by the "#red" command
void startRed(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    255,0,0  };
  TRACELN(F("set to red"));
}
// this function is invoked by the "#orange" command
void startOrange(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    255,128,0  };
  TRACELN(F("set to orange"));
}

// this function is invoked by the "#yellow" command
void startYellow(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    255,255,0  };
  TRACELN(F("set to yellow"));
}


// this function is invoked by the "#white" command
void startWhite(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    255,255,255  };
  TRACELN(F("set to white"));
}

// this function is invoked by the "#black" command
void startBlack(char* mess,int len){
  effectEngine.secondaryColor=effectEngine.primaryColor;
  effectEngine.primaryColor=(Color){
    0,0,0  };
  TRACELN(F("set to black"));
}


