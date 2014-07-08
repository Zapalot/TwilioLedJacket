///////////////////////////////////////////////////////////////////
//debug output that can be disabled but is still checked at compile time.
// This Code is in the public domain.
///////////////////////////////////////////////////////////////////

#pragma once
#include "Arduino.h"

#define TRACE(x) do { if (DEBUG) Serial.print( x); } while (0)
#define TRACELN(x) do { if (DEBUG) Serial.println( x); } while (0)

