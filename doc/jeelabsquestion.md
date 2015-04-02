http://jeelabs.net/projects/cafe/boards/6

Hi,


Im using JeeNodes in scenario where all nodes are equal, i.e. no master node.

Algorithms running on a node can sometimes take a while i.e. few seconds.

Message sent to node that is currently processing would result in an ack only after few seconds.

As shown here http://jeelabs.org/2011/01/12/relaying-rf12-packets/ common thing to do is to set up a timer and wait for ack but problem in this case is that the waiting time must be set very long.

What can be done, is immediate sending of the ack possible/recommended? Inside an interrupt or is there some other solution?


There are couple of questions considering ack:

Default idiom recommended in RF12.cpp to send an ack is this:
  `rf12_sendStart(RF12_ACK_REPLY, 0, 0);`

`RF12_ACK_REPLY` macro defined in `RF12.h` has following logic:
Node that is sending ack must identify itself in the ack. Without
using payload, node's ID has to be in driver's header rf12_hdr. To put
it there RF12_HDR_DST has to be set to 0.

This logic is leading to scenario in which ack is being broadcasted,
thus all other nodes will receive ack, although it is not for them.
Such ambiguous ack can lead node to have false notion that its package
has been delivered succesfully when it is not.

This situation is avoided by using protocol in which id of the node sending ack if needed is in payload so that ack can be set with destination node id in rf12_hdr.

This is noted in comments here http://jeelabs.org/2011/06/10/rf12-broadcasts-and-acks/#comments
