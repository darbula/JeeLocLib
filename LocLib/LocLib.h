// 2014-08-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php
#ifndef LocLib_h
#define LocLib_h


void showString(PGM_P s);
int freeRam();

// this is flag for debug macro defined in LocLib.cpp

#ifdef DEBUG_SERIAL
//#  define D(x) x
#  define DS(x) do { Serial.println(); Serial.print(millis()); Serial.print("ms: "); showString(PSTR(x)); } while (0)
#  define D(x) do { Serial.print(x); Serial.print(" "); } while (0)
#else
#  define DS(x)
#  define D(x)
#endif

#include <aoa_v2.h>
#include <globals.h>
#include <communication.h>
#include <ledstate.h>

#endif // DEBUG_SERIAL
