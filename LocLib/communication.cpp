// http://jeelabs.org/2011/12/10/inside-the-rf12-driver/
// http://jeelabs.org/2011/06/09/rf12-packet-format-and-design/
//#define DEBUG_SERIAL 1
#include <stdint.h>
#include <JeeLib.h>
#include <LocLib.h>
#include <stdlib.h> // rand, srand


/**
 * Protocol being used here demands usage of header and data in the payload.
 * It is best described with the following struct that should be defined in
 * program:
 * struct Message {
 *   // header defines:
 *   //   * type of the message allowing to have different structures for the
 *   //     data section
 *   //   * id of the sending node, reasoning for including it is given below
 *   //     beside code where ack is being sent
 *   uint8_t type;
 *   uint8_t source;
 *   // data structures matching different message types are defined as union
 *   // maximum struct size in this union should be RF12_MAXDATA - 2, i.e. 64
 *   union {
 *     struct tokenS token;
 *     struct aoAResponseS aoAResponse;
 *   };
 * };
 *
 */

/**
 * Message collision handling
 * --------------------------
 * Message collision that can happen in synchronised response to broadcast
 * such as AoAResponse is avoided by variable id-based send postponement.
 * There two scenarios:
 * 1. if node is expecting other messages while waiting to send then use
 * postponement with timer/alarm. Cost of this technique is memory used to
 * temporarily store data to send. Also, while waiting assert that no other
 * event should trigger sending new postponed message.
 * 2. if node is not expecting any incoming messages or alarms use blocking
 * delay sending without need for temporary storage.
 *
 * Message collision can happen in more generic case when there is no
 * synchronised responding. One example is LetUsStitch message sent whenever
 * node is ready to send it. In that case variable id based postponement is not
 * the solution. This problem is solved with acknowledgments and
 * random waiting resend postponements. Those postponement resends could be
 * accomplished in form of scenarios described above. For LetUsStitch the
 * blocking scenario is more appropriate i.e. simpler and feasible since during
 * blocking delay node is not expected to receive any message and all alarms
 * are disarmed. Sending with ack request and blocking delay is implemented in
 * send_req_ack().
 */

DisAlgProtocol::DisAlgProtocol (uint8_t node_id) : ID(node_id),
  // Resend delay based on 50kbps and maximum package of 75 byte that takes
  // 12ms should be around 20ms. Final resend delay is RESEND_DELAY_BASE*id
  RESEND_DELAY_BASE(20),
  // How many times should ack requesting message be resent value from maximum
  // node id in network to 255. Final number of resends is RESEND_COUNT_BASE/id
  RESEND_COUNT_BASE(200)
  // maximum time for send_req_ack is ~ RESEND_COUNT_BASE*RESEND_DELAY_BASE
  {
  }

DisAlgProtocol::DisAlgProtocol (uint8_t node_id, uint8_t resend_delay_base,
                                uint8_t resend_count_base) : ID(node_id),
                                RESEND_DELAY_BASE(resend_delay_base),
                                RESEND_COUNT_BASE(resend_count_base)
  {
  }

/**
 * Sends messages using RF12 interface. Function first checks if transmission
 * is possible and then sends prepared global g_message to destination node
 * id or if destination is 0 broadcasts. On successful transmission returns 1.
 */
void DisAlgProtocol::send(uint8_t type, uint8_t destination,
                          void* message, uint8_t len) {
  while (!rf12_canSend())
    rf12_recvDone();  // Ignores incoming messages
  //TODO: if message is received copy it set flag to prepare for processing

  *(uint8_t *)message = type;
  *((uint8_t *)message+1) = ID; // source
  memcpy((void *)rf12_data, message, len + 2);
  rf12_len = len + 2;
  if (destination!=0) {
    rf12_sendStart(RF12_HDR_DST | destination);
  } else {  // broadcast
    rf12_sendStart(ID);
  }
  DS("Sent: ");
  D((uint8_t)*rf12_data);
  DS(" to destination ");
  D(destination);
  DS(" ack wanted ");
  D(destination & RF12_HDR_ACK);
}

/**
 * Repeatedly send message with id based resend delay until ack is received.
 * Note that this function is currently using blocking delay and discarding all
 * incomming messages. Return 0 if failed;
 */
//TODO: make this function non blocking, get timer from the caller and return
uint8_t DisAlgProtocol::send_req_ack(uint8_t type, uint8_t destination,
                                     void* message, uint8_t len) {
  uint16_t c = 0;
  uint8_t ack_received=0;
  MilliTimer ackTimer;

  srand(ID);
  destination |= RF12_HDR_ACK;
  do {
    send(type, destination, message, len);
    ackTimer.set(RESEND_DELAY_BASE*(rand() % 10 + 1));
    while(!ack_received && !ackTimer.poll()) {
      ack_received = ((rf12_recvDone() && rf12_crc==0) &&  // received
                      (rf12_hdr & RF12_HDR_CTL) &&         // is ack
                      (*(uint8_t *)rf12_data==type) &&     // same type
                      (*((uint8_t *)rf12_data+1)==(destination & RF12_HDR_MASK)) && // from the right node
                      ((rf12_hdr & RF12_HDR_MASK)==ID)); // for this node

      if (ack_received) {
        DS("Received ack");
        /*
        D(ack_received);
        D(rf12_hdr);
        D(rf12_hdr & RF12_HDR_CTL);
        D(*(uint8_t *)rf12_data);
        D(type);
        D((*((uint8_t *)rf12_data+1) & RF12_HDR_MASK));
        D(destination);
        D(rf12_hdr & RF12_HDR_MASK);
        D(ID);
        */
      }
    };
    c++;
  } while (!ack_received && c<RESEND_COUNT_BASE);
  return c<RESEND_COUNT_BASE;
}

/**
 * Returns 1 if the message is received and CRC is correct or 0 if not.
 */
uint8_t DisAlgProtocol::receive(void* message) {
  // Driver is checking if destination stated in rf12_hdr is appropriate inside
  // rf12_recvDone. If it returns 1 destination should be either this node id
  // or 0 for broadcast, or if this node id is 31 then it can be 0 to 31.
  if (rf12_recvDone()) {
    if (rf12_crc!=0) {
      DS("Received: CRC fail ");
      return 0;
    }
    // immediately copy all data from rf12_buf to be able to release receiver
    memcpy(message, (void*)rf12_data, rf12_len);
    uint8_t rf12_wants_ack = RF12_WANTS_ACK;
    // destination currently not used
    //uint8_t destination = *(uint8_t *)rf12_hdr;

    DS("Received: hdr ");
    D(rf12_hdr);

    // Driver is in TXIDLE so until rf12_recvDone() is called again receiver
    // is shut down. This is done to prevent writing to volatile rf12_data but
    // since all data is already copied it is safe to release receiver:
    rf12_recvDone();

    DS(" Payload type: ");
    D(*(uint8_t *)message);
    DS(" Payload source: ");
    D(*((uint8_t *)message+1));

    //To use Collect, bit 0x20 in Node Id, see RF12demo.ino
    //if (RF12_WANTS_ACK && (id & COLLECT) == 0) {
    if (rf12_wants_ack) {
      // Default idiom recommended in RF12.cpp to send an ack is this:
      //   rf12_sendStart(RF12_ACK_REPLY, 0, 0);
      // RF12_ACK_REPLY macro defined in RF12.h has following logic:
      // Node that is sending ack must identify itself in the ack. Without
      // using payload, node's ID has to be in driver's header rf12_hdr. To put
      // it there RF12_HDR_DST has to be set to 0.
      // This logic is leading to scenario in which ack is being broadcasted,
      // thus all other nodes will receive ack, although it is not for them.
      // Such ambiguous ack can lead node to have false notion that its package
      // has been delivered succesfully when it is not.
      // This situation is avoided by using protocol in which node's id is in
      // payload so that ack can be set with destination node id in rf12_hdr.

      // construct header for ack with same type and this node id as source
      uint8_t header[2] = {*(uint8_t *)message, ID};
      rf12_sendStart(RF12_HDR_CTL | RF12_HDR_DST | *((uint8_t *)message+1), &header, sizeof(header));
      DS("Sending ack:");
      DS(" hdr: ");
      D(RF12_HDR_CTL | RF12_HDR_DST | *((uint8_t *)message+1));
      DS(" Payload type: ");
      D(header[0]);
      DS(" Payload source: ");
      D(header[1]);
    }

    return 1;
  } else {
    // Nothing is received or CRC doesn’t match
    return 0;
  }
}
