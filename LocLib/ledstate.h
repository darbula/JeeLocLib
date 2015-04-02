// 2014-08-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php

#ifndef ledstate_h
#define ledstate_h

/// @file
/// ledstate library definitions.

#include <JeeLib.h>

/// Interface for the LedState Plug
class LedStatePlug {
    BlinkPlug port1;
    BlinkPlug port2;

public:
    LedStatePlug (byte, byte);

    void set(uint8_t);
};

#endif // ledstate_h
