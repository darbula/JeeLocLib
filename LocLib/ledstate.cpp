/// @file
/// ledstate library definitions.
// 2014-09-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php

#include <ledstate.h>
#include <JeeLib.h>


/**
 * LedState uses two ports (p1, p2) it should be initialized
 * with (1, 4) or (2, 3)
 */
LedStatePlug::LedStatePlug (byte p1, byte p2) :
                  port1(BlinkPlug(p1)), port2(BlinkPlug(p2)) {
  checkFlags = 0;
  currentState = 0;
}


/** Set leds.
 *  @param state Number in which last four bits set the leds
 */
void LedStatePlug::set(uint8_t state) {
  if (state & 0x01) port1.ledOn(1);
  else port1.ledOff(1);
  if (state & 0x02) port1.ledOn(2);
  else port1.ledOff(2);
  if (state & 0x04) port2.ledOn(1);
  else port2.ledOff(1);
  if (state & 0x08) port2.ledOn(2);
  else port2.ledOff(2);
}

/** Check the state of the buttons.
 *  @return The corresponding enum state: ON1, OFF1, ON2, or OFF2.
 */
byte LedStatePlug::buttonCheck() {
  // delegate events detection to blink plug code
  byte event1 = port1.buttonCheck();  // 0, 1, or 2
  if (event1==BlinkPlug::ON1 || event1==BlinkPlug::OFF1) {
    bitSet(checkFlags, event1);
    if (event1==BlinkPlug::ON1) bitSet(currentState, 0);
    else bitClear(currentState, 0);
  }
  byte event2 = port2.buttonCheck();
  if (event2==BlinkPlug::ON1 || event2==BlinkPlug::OFF1) {
    bitSet(checkFlags, event2+2);
    if (event2==BlinkPlug::ON1) bitSet(currentState, 1);
    else bitClear(currentState, 1);
  }
  // note that simultaneous button events will be returned in successive calls
  if (checkFlags) {
    for (byte i = BlinkPlug::ON1; i <= BlinkPlug::OFF2; ++i) {
      if (bitRead(checkFlags, i)) {
        bitClear(checkFlags, i);
        return i;
      }
    }
  }
  // if there are no button events, return the overall current button state
  return currentState == 3 ? BlinkPlug::ALL_ON : currentState ? BlinkPlug::SOME_ON : BlinkPlug::ALL_OFF;
}

