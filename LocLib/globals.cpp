#include <stdint.h> // uint8_t
#include <stddef.h> // NULL
#include <macros_structs.h>
#include "globals.h"



uint8_t g_id = 0; // node id
uint8_t g_group = 0;
uint8_t g_status = 0; //current node status


// contains ids
// g_neighbors[0] is g_id, set up in algorithm
uint8_t g_neighbors[MAX_NODES] = {0};
uint8_t g_num_neighbors = 0;
uint8_t g_done_children[MAX_NEIGHBORS] = {0};
uint8_t g_num_done_children = 0;
uint8_t g_children_to_forward[MAX_NEIGHBORS] = {0};
uint8_t g_num_children_to_forward = 0;
float g_jbest_orientation = 0;
uint8_t g_iter_orientation = 0;
float g_jbest_angulation = 0;
uint8_t g_iter_angulation = 0;
Tree g_tree = {0};

// aoa measurements vector
#ifdef USE_HEAP
AoAMeasurement* g_aoa=NULL;
#else
AoAMeasurement g_aoa[MAX_AOA_MEASUREMENTS];
#endif // USE_HEAP
uint8_t g_num_aoa = 0;

// aoa measurements matrix
float g_fi[MAX_NEIGHBORS][MAX_NEIGHBORS];
// neighbors estimated orientations
float g_alpha_sensors[MAX_NEIGHBORS];
float g_alpha_sensors_tmp[MAX_NEIGHBORS];

// - the result of multiorientation
// - the argument of multiangulation
#ifdef USE_HEAP
MeasureVector* g_measurements=NULL;
#else
MeasureVector g_measurements[MAX_MEASUREMENTS];
#endif // USE_HEAP

// number of positions is equal to g_num_neighbors
Position g_neighbors_positions[MAX_NEIGHBORS];
Position g_neighbors_positions_tmp[MAX_NEIGHBORS];

// Final positions
#ifdef USE_HEAP
Position* g_destination_cluster=NULL;
#else
Position g_destination_cluster[MAX_NODES];
#endif // USE_HEAP
uint8_t g_num_destination;

// temporary structure for positions received in LetUsStitch message
#ifdef USE_HEAP
Position* g_source_cluster=NULL;
#else
Position g_source_cluster[MAX_NODES];
#endif // USE_HEAP
uint8_t g_num_source;


/*

 Memory usage

Var name                     size in bytes             1  2p 2o 2a 3p 3s  heap
-------------------------------------------------------------------------------
g_id                         1          1        1     O--O--O--O--O--O-
g_status                     1          1        1     O--O--O--O--O--O-

g_message (communication.h) 66         66       66     O--O--O--O--O--O-

g_neighbors                  NOD       16       16     O--O--O--O--O--O-
g_num_neighbors              1          1        1     O--O--O--O--O--O-
g_done_children              NEI        7        7     O--------------O-
g_num_done_children          1          1        1     O--------------O-
g_children_to_forward        NEI        7        7     O--------------O-
g_num_children_to_forward    1          1        1     O----------------
g_jbest_orientation          4          4        4     ------O----------
g_iter_orientation           4          4        4     ------O----------
g_jbest_angulation           4          4        4     ---------O-------
g_iter_angulation            4          4        4     ---------O-------
g_tree                       Tree       2+NEI    9     O--O--O--O--O--O-
-------------------------------------------------------------------------------
Fixed:                                         126

g_aoa;                       AoA*MEA    4*112  448     O--O-------------  1,2p
g_num_aoa                    1                   1     O--O-------------

g_fi                         4*NEI^2    4*7*7  196     ---O--O----------

g_alpha_sensors              4*NEI      4*7     28     ------O----------
g_alpha_sensors_tmp          4*NEI      4*7     28     ------O----------

g_measurements         MeasV*NEI^2-NEI 10*6*7  420     ------O--O-------  2o,2a

g_neighbors_positions_tmp    Pos*NEI    9*7     63     ---------O-------
g_neighbors_positions        Pos*NEI    9*7     63     ---------O--O----

g_destination_cluster        Pos*NOD    9*16   144     ------------O--O-  3p,3s
g_num_destination            1          1        1     ------------O--O-

g_source_cluster             Pos*NOD    9*16   144     ---------------O-  3p,3s
g_num_source                 1          1        1     ---------------O-
-------------------------------------------------------------------------------
                                          tot 1575
                                           max 930
                                               743 data?
                                               210 stack?
                                               132 free via freeRam
                                              2015
MAX_NODES             NOD     16
MAX_NEIGHBORS         NEI     7
MAX_AOA_MEASUREMENTS  MEA     NOD*NEI 7*16 = 112

Tree              Tree    (1+1+7)  9
AoAMeasurement    AoA     (1+1+2)  4
MeasureVector     MeasV   (1+1+8)  10
Position          Pos     (1+8)    9

Fixed:   101
Fixed x: 381
         482

Phases:
  1 measurement = 448 + 482 = 930
  2 prepare     = 448 + 482 = 930
  2 orientation = 420 + 482 = 902
  2 angulation  = 420 + 482 = 902
  3 prepare     = 288 + 482 = 770
  3 stitch      = 288 + 482 = 770

*/
