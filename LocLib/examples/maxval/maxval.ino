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

#define PROTOCOL_EEPROM_ADDR (uint8_t*)0x320
/*
 * byte  0x320  RESEND_DELAY_BASE         Protocol param, communications.cpp
 * byte  0x321  RESEND_COUNT_BASE           "
 */

#define OUTBOX_QUEUE_SIZE 10
#define MAX_NODES 16

// Node statuses
enum {
  IDLE, HASVAL, NOTMAX, MAX
};

// Message types
enum {
  Reset,
  Sync,
  SetStatus,
  SetNeighbors,
  Value,
  MaxVal,
};

struct setstatusS {
  uint8_t status;
};

struct setneighborsS {
  uint8_t neighbors[MAX_NODES];
};

struct valueS {
  uint8_t val;
};

struct maxvalS {
  uint8_t val;
};

struct Message {
  uint8_t header;
  uint8_t source;
  union {
    struct setstatusS setstatus;
    struct setneighborsS setneighbors;
    struct valueS value;
    struct maxvalS maxval;
  };
};

Message g_message;

struct OutboxQueue {
  Message messages[OUTBOX_QUEUE_SIZE];
  int destinations[OUTBOX_QUEUE_SIZE];
  int headers[OUTBOX_QUEUE_SIZE];
  int lengths[OUTBOX_QUEUE_SIZE];
  uint8_t head;
  uint8_t tail;
};

OutboxQueue outbox_queue = {
  .messages={},
  .destinations={},
  .headers={},
  .lengths={},
  .head=0,
  .tail=0
};

MilliTimer processOutboxTimer;

// jeenode ports where ledstate is plugged
LedStatePlug ledstate(2, 3);

DisAlgProtocol rf(eeprom_read_byte(ID_EEPROM_ADDR) & 0x1F,
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR),
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR+1));

void setup();
void loop();
void reset();
void set_status(uint8_t);
uint8_t update_maxval(uint8_t);
uint8_t pushOutboxQueue(uint8_t, uint8_t, void*, uint8_t);
void processOutbox();

// register containing value
uint8_t g_val;
uint8_t g_maxval;

uint8_t g_neighbors[MAX_NODES];

void setup() {
  g_id = eeprom_read_byte(ID_EEPROM_ADDR) & 0x1F;
  g_group = eeprom_read_byte(GROUP_EEPROM_ADDR);

#ifdef DEBUG_SERIAL
  Serial.begin(57600);
  // set timeout for parseInt command
  Serial.setTimeout(10000);
  DS("[maxval]");
  DS("Node id: ");
  D(g_id);
  DS("Node group: ");
  D(g_group);
#endif // DEBUG_SERIAL

  reset();
  rf12_initialize(g_id, RF12_868MHZ, g_group);
}

void reset() {
  // initialization light show
  for (int i = 0; i < 16; ++i) {
    set_status(i);
    delay(100);
  }
  outbox_queue.head = outbox_queue.tail = 0;
  set_status(IDLE);
}

// MAIN FUNCTION
/*
 * Listen incoming messages via rf on node and act and respond
 * according to their header, that is, first three bits of rf12_data.
 */
void loop() {
  // ************************* Messages *************************
  if (rf.receive(&g_message) && (are_neighbors(g_id, g_message.source) || g_message.source==31)) {
    DS("Message received: ");
    switch (g_message.header) {
      case Reset:
        DS("Reset");
        reset();
        break;
      case Sync:
        DS("Sync");
        processOutboxTimer.set(g_id*20);
        break;
      case SetStatus:
        DS("SetStatus status=");
        D(g_message.setstatus.status);
        set_status(g_message.setstatus.status);
        break;
      case SetNeighbors:
        DS("SetNeighbors neighbors=");
        for (int i = 0; i < MAX_NODES; ++i) {
          D(g_message.setneighbors.neighbors[i]);
        }
        memcpy(g_neighbors, g_message.setneighbors.neighbors, MAX_NODES);
        for (int i = 0; i < MAX_NODES; ++i) {
          D(g_neighbors[i]);
        }
        break;
      case Value:
        DS("Value val=");
        D(g_message.value.val);
        g_val = g_message.value.val;
        g_maxval = g_val;
        set_status(HASVAL);
        break;
      case MaxVal:
        // to send use pushOutboxQueue(header, destination, &g_message, length);
        DS("MaxVal ");
        switch (g_status) {
          case IDLE:
          case HASVAL:
            if (update_maxval(g_message.maxval.val)) {
              set_status(NOTMAX);
            } else {
              set_status(MAX);
            }
            g_message.maxval.val = g_maxval;
            pushOutboxQueue(MaxVal, 0, &g_message, sizeof(maxvalS));
            break;
          case NOTMAX:
          case MAX:
            if (update_maxval(g_message.maxval.val)) {
              pushOutboxQueue(MaxVal, 0, &g_message, sizeof(maxvalS));
              set_status(NOTMAX);
            }
            break;
        }
        break;
      default:
        DS("Header not recognized");
        break;
    }
  }

  // ********************** Spontaneously ***********************
  if (ledstate.buttonCheck()==BlinkPlug::OFF1) {
    g_message.maxval.val = g_maxval;
    rf.send(MaxVal, 0, &g_message, sizeof(maxvalS));
    set_status(MAX);
  }


  // ********************** Timers/Alarms ***********************
  if (processOutboxTimer.poll()) {
    processOutboxQueue();
  }
}

// PROCEDURES

uint8_t pushOutboxQueue(uint8_t header, uint8_t destination, void *message, uint8_t length) {
  if ((outbox_queue.tail+1)%OUTBOX_QUEUE_SIZE==outbox_queue.head) {
    DS("Outbox queue full.");
    return 0;
  }
  outbox_queue.destinations[outbox_queue.tail] = destination;
  outbox_queue.headers[outbox_queue.tail] = header;
  outbox_queue.lengths[outbox_queue.tail] = length;
  memcpy(&outbox_queue.messages[outbox_queue.tail], message, sizeof(g_message));
  outbox_queue.tail = (outbox_queue.tail+1)%OUTBOX_QUEUE_SIZE;
  return 1;
}

// If there are messagess in the outbox send first one
void processOutboxQueue() {
  if (outbox_queue.head!=outbox_queue.tail) {
    rf.send(outbox_queue.headers[outbox_queue.head], outbox_queue.destinations[outbox_queue.head],
         &outbox_queue.messages[outbox_queue.head], outbox_queue.lengths[outbox_queue.head]);
    outbox_queue.head = (outbox_queue.head+1)%OUTBOX_QUEUE_SIZE;
  }
}

uint8_t update_maxval(uint8_t val) {
  if (g_maxval < val) {
    g_maxval = val;
    return 1;
  }
  return 0;
}
void set_status(uint8_t s) {
  g_status = s;
  ledstate.set(s);
}

uint8_t are_neighbors(uint8_t id1, uint8_t id2) {
  uint8_t ret = 0;
  for (int i = 0; i < MAX_NODES; ++i) {
    ret |= g_neighbors[i] && id2==g_neighbors[i];
    D(id2);
    D(g_neighbors[i]);
  }
  DS("ret");
  D(ret);
  return ret;
  /*
  // chain
  return (id1-id2)==1 || (id2-id1)==1;
  */
  /*
  // hypercube
  int x = id1^id2;
  int sum = 0;
  for (int i = 0; i < 4; ++i) sum += (x>>i)&1;
  return sum==1;
  */
}
