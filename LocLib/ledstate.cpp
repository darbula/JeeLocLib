/// @file
/// ledstate library definitions.
// 2014-09-01 <damir.arbula@gmail.com> http://opensource.org/licenses/mit-license.php

#include <ledstate.h>
#include <JeeLib.h>


/**
 * LedState uses two ports: num should be 1 or 2, it is the port where the
 * phototransistor side is connected, hint: just use 1 and connect accordingly
 */
LedStatePlug::LedStatePlug (byte p1, byte p2) :
                  port1(BlinkPlug(p1)), port2(BlinkPlug(p2)) {
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
