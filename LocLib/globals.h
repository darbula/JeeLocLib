#include <stdint.h> // uint8_t
#include <stddef.h> // NULL
#include <macros_structs.h>

#ifndef GLOBALS_H
#define GLOBALS_H

// see globals.cpp for memory usage analisys
#define USE_HEAP 1

extern uint8_t g_id;
extern uint8_t g_group;
extern uint8_t g_status;
// g_neighbors[0] = g_id
extern uint8_t g_neighbors[MAX_NODES];
extern uint8_t g_num_neighbors;
extern uint8_t g_done_children[MAX_NEIGHBORS];
extern uint8_t g_num_done_children;
extern uint8_t g_children_to_forward[MAX_NEIGHBORS];
extern uint8_t g_num_children_to_forward;
extern float g_jbest_orientation;
extern uint8_t g_iter_orientation;
extern float g_jbest_angulation;
extern uint8_t g_iter_angulation;

struct Tree {
  uint8_t parent;
  uint8_t children[MAX_NEIGHBORS];
  uint8_t num_children;
};

extern Tree g_tree;

#ifdef USE_HEAP
extern AoAMeasurement* g_aoa;
#else
extern AoAMeasurement g_aoa[MAX_AOA_MEASUREMENTS];
#endif // USE_HEAP
extern uint8_t g_num_aoa;

// aoa measurements in r(eceiver) node local coordinate system i.e. in generic
// case g_fi[r,t] != g_fi[r,t] +- PI. If no measurement is taken or if the
// element is on main diagonal value is set to INF
extern float g_fi[MAX_NEIGHBORS][MAX_NEIGHBORS];
// neighbors azimuths in node coordinate system
extern float g_alpha_sensors[MAX_NEIGHBORS];
extern float g_alpha_sensors_tmp[MAX_NEIGHBORS];

// aoa measurements array in node coordinate system in form of sin/-cos vectors
#ifdef USE_HEAP
extern MeasureVector* g_measurements;
#else
extern MeasureVector g_measurements[MAX_MEASUREMENTS];
#endif // USE_HEAP
extern uint8_t g_num_measurements;

// multiangulation global variables
extern Position g_neighbors_positions[MAX_NEIGHBORS];
extern Position g_neighbors_positions_tmp[MAX_NEIGHBORS];

// Stich global variables
#ifdef USE_HEAP
extern Position* g_destination_cluster;
#else
extern Position g_destination_cluster[MAX_NODES];
#endif // USE_HEAP
extern uint8_t g_num_destination;

#ifdef USE_HEAP
extern Position* g_source_cluster;
#else
extern Position g_source_cluster[MAX_NODES];
#endif // USE_HEAP
extern uint8_t g_num_source;

#endif // GLOBALS_H
