//
// dftraversal.ino
//

#include <JeeLib.h>
#include <LocLib.h>

/**
 * Maximum number of nodes.
 */
#define MAX_NODES 32

/**
 * Time delay in miliseconds.
 * Used when sending NeighborConfirm messages to avoid collision.
 * Also used when waiting for all (neighbor) nodes to respond.
 */
#define NEIGHBOR_DETECT_POSTPONEMENT 100

/**
 * Node's permanent configuration settings for this program is set in EEPROM
 * with following layout:
 */
#define ID_EEPROM_ADDR (uint8_t*)0x020
#define GROUP_EEPROM_ADDR (uint8_t*)0x021
/*
 * New settings in line wit those in rf12Config:
 * byte  0x020  NodeId 1 - 16
 * byte  0x021  GroupId
 */

#define PROTOCOL_EEPROM_ADDR (uint8_t*)0x320
/*
 * byte  0x320  RESEND_DELAY_BASE         Protocol param, communications.cpp
 * byte  0x321  RESEND_COUNT_BASE           "
 */

#define READ_NODE_ID (eeprom_read_byte(ID_EEPROM_ADDR) & 0x1F)
#define READ_GROUP_ID (eeprom_read_byte(GROUP_EEPROM_ADDR))

enum NodeState
{
	DUMMY,
	IDLE,
	INITIATOR,
	VISITED,
	DONE,
};

enum MessageType
{
	Spontaneously,
	TraversalToken,
	TraversalReturn,
	TraversalBackedge,
	TraversalForwardedge,
	NeighborDetect,  // "Are you my neighbor?"
	NeighborConfirm,  // "Yes, I am your neighbor!"
};

struct Message
{
	uint8_t header;
	uint8_t source;
};

Message g_message;

MilliTimer listenTimer;

LedStatePlug ledstate(2,3);  // JeeNode ports where LEDState plug is plugged

DisAlgProtocol rf(READ_NODE_ID,
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR),
                  eeprom_read_byte(PROTOCOL_EEPROM_ADDR+1));

MilliTimer neighborDetectionTimer;

boolean g_neighbor_detection_initiated;
boolean g_neighbor[MAX_NODES];
boolean g_visited[MAX_NODES];

boolean g_initiator;

int g_entry;

void reset();
void listen_rf_loc();
void visit();
uint8_t get_next_unvisited();
void start_neighbor_detection();
void set_status(uint8_t);

void setup()
{
	g_id = READ_NODE_ID;
	g_group = READ_GROUP_ID;

	rf12_initialize(g_id, RF12_868MHZ, g_group);

	reset();
}

void loop()
{
	listen_rf_loc();
}

void reset()
{
	for (int i = 0; i < MAX_NODES; i++)
	{
		g_neighbor[i] = false;
		g_visited[i] = false;
	}

	g_neighbor_detection_initiated = false;

	set_status(IDLE);
}

void listen_rf_loc()
{
	while(1)
	{
		if (rf.receive(&g_message))
		{
			switch (g_message.header)
			{
			case NeighborDetect:
				// Send with postponement
				delay(NEIGHBOR_DETECT_POSTPONEMENT * g_id);
				rf.send(NeighborConfirm, g_message.source, &g_message, 0);
				break;
			case NeighborConfirm:
				// Mark message sender as neighbor
				g_neighbor[g_message.source] = true;
				break;
			}  // switch (g_message.header)

			switch (g_status)
			{
			case INITIATOR:
				break;
			case IDLE:
				switch (g_message.header)
				{
				case Spontaneously:
					g_initiator = true;  // Set flag
					set_status(INITIATOR);
					start_neighbor_detection();
					break;
				case TraversalToken:
					g_initiator = false;
					g_entry = g_message.source;
					g_visited[g_message.source] = true;
					start_neighbor_detection();
					break;
				}
				break;
			case VISITED:
				switch (g_message.header)
				{
				case TraversalToken:
					g_visited[g_message.source] = true;
					rf.send(TraversalBackedge, g_message.source, &g_message, 0);
					break;
				case TraversalReturn:
				case TraversalBackedge:
				case TraversalForwardedge:
					visit();
					break;
				}
				break;
			case DONE:
				switch (g_message.header)
				{
				case TraversalToken:
					rf.send(TraversalForwardedge, g_message.source, &g_message, 0);
					break;
				}
				break;
			}  // switch (g_status)
		}  // if (rf.receive(&g_message))

		if (neighborDetectionTimer.poll())
		{
			visit();
		}  // if (neighborDetectionTimer.poll())
	}  // while (1)
}

void visit()
{
	uint8_t next = get_next_unvisited();
	if (next != 0xFF)
	{
		g_visited[next] = true;

		rf.send(TraversalToken, next, &g_message, 0);

		set_status(VISITED);
	}
	else
	{
		if (!g_initiator)
		{
			rf.send(TraversalReturn, g_entry, &g_message, 0);
		}

		set_status(DONE);
	}
}

uint8_t get_next_unvisited()
{
	for (int i = 0; i < MAX_NODES; i++)
	{
		if (g_neighbor[i] && !g_visited[i])
		{
			return i;
		}
	}

	return 0xFF;  // Means that there are no (more) unvisited neighbors
}

void start_neighbor_detection()
{
	if (!g_neighbor_detection_initiated)
	{
		rf.send(NeighborDetect, 0, &g_message, 0);  // Brodacast NeighborDetect message

		g_neighbor_detection_initiated = true;

		neighborDetectionTimer.set(NEIGHBOR_DETECT_POSTPONEMENT * MAX_NODES);
	}
}

void set_status(uint8_t new_status)
{
	g_status = new_status;
	ledstate.set(new_status);
}
