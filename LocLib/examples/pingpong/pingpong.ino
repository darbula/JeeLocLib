#define DEBUG_SERIAL 1
#include <avr/eeprom.h>
#include <JeeLib.h>
#include <LocLib.h>

/*
 * Node's permanent configuration settings for this program is set in EEPROM
 * with following layout:
 */
#define ID_EEPROM_ADDR (uint8_t*)0x020
#define GROUP_EEPROM_ADDR (uint8_t*)0x021

/* New settings in line with those in rf12Config:
 * byte  0x020  NodeId 1 - 16
 * byte  0x021  GroupId
 */

/* Address defined in aoa_v2.cpp
 * byte  0x100  AoA Calibration Params    See aoa_v2.h
 * byte  0x17B    "
 * word  0x17C  MAX_VAL_THRESHOLD
 */

#define PROTOCOL_EEPROM_ADDR (uint8_t*)0x320
/*
 * byte  0x320  RESEND_DELAY_BASE         Protocol param, communications.cpp
 * byte  0x321  RESEND_COUNT_BASE           "
 */


// Node statuses
enum {
  IDLE, PING, PONG
};

// Message types
enum {
  Reset, // Control
  Token,
};

struct tokenS {
  uint8_t counter;
};

struct Message {
  uint8_t header;
  uint8_t source;
  union {
    struct tokenS token;
  };
};

Message g_message;

MilliTimer listenTimer;

// jeenode ports where ledstate is plugged
LedStatePlug ledstate(2, 3);

DisAlgProtocol rf(eeprom_read_byte(ID_EEPROM_ADDR) & 0x1F,
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR),
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR+1));

void setup();
void loop();
void reset();
void serial_control(char u);
void help();
void listen_rf_loc();
void set_status(uint8_t s);

void setup() {
  g_id = eeprom_read_byte(ID_EEPROM_ADDR) & 0x1F;
  g_group = eeprom_read_byte(GROUP_EEPROM_ADDR);

#ifdef DEBUG_SERIAL
  Serial.begin(57600);
  // set timeout for parseInt command
  Serial.setTimeout(10000);
  DS("[pingpong]");
  DS("Node id1: ");
  D(g_id);
  help();
  // wait before switching into listening mode
  listenTimer.set(3000);
#endif // DEBUG_SERIAL

  reset();
  rf12_initialize(g_id, RF12_868MHZ, g_group);
}

void reset() {
  set_status(IDLE);
}

void loop() {
#ifdef DEBUG_SERIAL
  if (Serial.available()){
    serial_control(Serial.read());
    listenTimer.set(0); // disarm timer
  }
  if (listenTimer.poll()){
    serial_control('r');
  }
#else
  listen_rf_loc();
#endif // DEBUG_SERIAL
}

#ifdef DEBUG_SERIAL
void serial_control(char u) {
  uint16_t val;
  uint8_t destination;

  switch (u) {
    default:
      help();
      break;
    case 's':
      DS("Destination:");
      destination = Serial.parseInt();
      D(destination);
      DS("Counter:");
      g_message.token.counter = Serial.parseInt();
      D(g_message.token.counter);
      DS("Sending:");
      //rf.send_req_ack(Token, destination, &g_message, sizeof(tokenS));
      rf.send(Token, destination, &g_message, sizeof(tokenS));
      DS("Sent.");
      break;
    case 'r':
      DS("Entering listening mode, reset node to exit.");
      listen_rf_loc();
      break;
   }
}
#endif // DEBUG_SERIAL

/**
 * Help messages.
 */
void help() {
  DS("Select:");
  DS("r - for activating listening mode");
  DS("s - send token to some node");
}

// MAIN FUNCTION
/*
 * Listen incoming messages via rf on node and act and respond
 * according to their header, that is, first three bits of rf12_data.
 */
void listen_rf_loc() {
  while(1) {
    // Messages
    if (rf.receive(&g_message)) {
      DS("Message received: ");
      switch (g_message.header) {
        case Reset:
          DS("Reset");
          reset();
          break;
        case Token:
          DS("Token");
          DS("Token counter: ");
          D(g_message.token.counter);
          switch (g_status) {
            case IDLE:
            case PING:
              g_message.token.counter++;
              DS("Status Ping");
              delay(1000);
              rf.send(Token, g_message.source, &g_message, 1);
              set_status(PONG);
              break;
            case PONG:
              g_message.token.counter++;
              DS("Status Pong");
              delay(1000);
              rf.send(Token, g_message.source, &g_message, 1);
              set_status(PING);
              break;
          }
          break;
      }
    }

    // ********************** Spontanously ************************
    if (ledstate.buttonCheck()==BlinkPlug::OFF1) {
      rf.send(Token, 0, &g_message, 1);
    }


    // ********************** Timers/Alarms ***********************
    /*
    if (someTimer.poll()) {
      doSomething();
    }
    */
  }
}

// PROCEDURES
void set_status(uint8_t s) {
  g_status = s;
  ledstate.set(s);
}
