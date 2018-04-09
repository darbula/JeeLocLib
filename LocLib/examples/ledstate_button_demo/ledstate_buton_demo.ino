#include <JeeLib.h>
#include <LocLib.h>

// jeenode ports where ledstate is plugged
LedStatePlug ledstate(2, 3);
MilliTimer everySecond;

void setup () {
  Serial.begin(57600);
  Serial.println("\n[ledstate_button_demo]");
}

void loop () {
  byte event = ledstate.buttonCheck();
  switch (event) {

  case BlinkPlug::ON1:
    Serial.println("  Button 1 pressed");
    break;

  case BlinkPlug::OFF1:
    Serial.println("  Button 1 released");
    break;

  case BlinkPlug::ON2:
    Serial.println("  Button 2 pressed");
    break;

  case BlinkPlug::OFF2:
    Serial.println("  Button 2 released");
    break;

  default:
    // report these other events only once a second
    if (everySecond.poll(1000)) {
      switch (event) {
        case BlinkPlug::ALL_ON:
          Serial.println("ALL buttons are currently pressed");
          break;
        case BlinkPlug::SOME_ON:
          Serial.println("SOME button is currently pressed");
          break;
        case BlinkPlug::ALL_OFF:
          Serial.println("NO buttons are currently pressed");
          break;
      }
    }
  }
}
