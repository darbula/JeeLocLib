#include <JeeLib.h>



// http://jeelabs.org/2011/05/23/saving-ram-space/
void showString (PGM_P s) {
  char c;
  while ((c = pgm_read_byte(s++)) != 0){
    Serial.print(c);
  }
}

// http://jeelabs.org/2011/05/22/atmega-memory-use/
// http://www.nongnu.org/avr-libc/user-manual/malloc.html
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
