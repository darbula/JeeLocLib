// Fcuntions for debugging
// The function is controlled by the DEBUG define. A value of 1 just enables
// LED blinking, a value of 2 enables serial printing and disables the LED

//====== LED BLINKS ======
#if DEBUG & 1
#define LED_DDR     DDRD
#define LED_PORT    PORTD
#define LED_PIN     PIND
#define LED         PIND5 // (JeeNode Port2 DIO)

// Watchdog functions. These are only safe with interrupts turned off.
static void watchdogReset() {
  __asm__ __volatile__ (
    "wdr\n"
  );
}

static void debug_flash_led(uint8_t count) {
  do {
    timer_start(200);
		while(!timer_done())
			;
    LED_PIN |= _BV(LED);
    watchdogReset();
  } while (--count);
}

#else
#define debug_flash_led(x)
#endif

//====== SERIAL PRINTING ======
#if DEBUG & 2

// UART STUFF
#define BAUD_RATE 57600L
# define UART_SRA UCSR0A
# define UART_SRB UCSR0B
# define UART_SRC UCSR0C
# define UART_SRL UBRR0L
# define UART_UDR UDR0

// print character
static void putch(char ch) {
  while (!(UART_SRA & _BV(UDRE0)));
  UART_UDR = ch;
}
// print string
static void DEBUG_P(char *str) {
	while (*str) putch(*str++);
}
// print newline
static inline void P_LN(void) { putch('\n'); }
// print byte in hex
static void DEBUG_P_X8(uint8_t v) {
	uint8_t vh = v>>4;
	putch(vh>9 ? vh+'a'-10 : vh+'0');
	uint8_t vl = v & 0xf;
	putch(vl>9 ? vl+'a'-10 : vl+'0');
}
// print word in hex
static void DEBUG_P_X16(uint16_t v) {
	P_X8(v>>8);
	P_X8(v&0xFF);
}
// print array of bytes
static void DEBUG_P_A(void *arr, uint8_t n) {
	uint8_t *v = arr;
	P_X16((uint16_t)v);
	P(": ");
	while (n--) {
		P_X8(*v++); putch(' ');
	}
	P_LN();
}

#else
#define DEBUG_P(...)
#define DEBUG_P_X8(...)
#define DEBUG_P_X16(...)
#define DEBUG_P_A(...)
#define DEBUG_P_LN(...)
#endif
