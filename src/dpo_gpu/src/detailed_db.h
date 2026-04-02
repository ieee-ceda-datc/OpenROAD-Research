#include <torch/extension.h>

#include <vector>
#include <iostream>
#include <tuple>
#include <string>

#include "architecture.h"
#include "detailed.h"
#include "detailed_manager.h"
#include "legalize_shift.h"
#include "network.h"
#include "orientation.h"
#include "router.h"
#include "symmetry.h"

namespace dpo {

class DetailedPlaceDB {
public:
  DetailedPlaceDB(Network* network, Architecture* arch) {}

  ~DetailedPlaceDB();

  void createDetailedPlaceDB();

  Network* network_;
  Architecture* arch_;
  DetailedMgr* mgr_;

  /* chip info */
  float xl;
  float yl;
  float xh;
  float yh;
  int numNodes;         // number of cells in netlist
  int numPins;          // number of pins in netlist  
  int numEdges;         // number of nets in netlist
  int numRegions;       // number of regions
  int numMovableNodes;  // number of movable nodes (single height cells)

  float* x;
  float* y;
  const float* initX;
  const float* initY;
  const float* nodeSizeX;
  const float* nodeSizeY;
  //const float* nodeCX;
  //const float* nodeCY;
  torch::Tensor nodeSizeTensor;

  const float* pinOffsetX;
  const float* pinOffsetY;

  const int* flat_node2pin_start_map;
  const int* flat_node2pin_map;
  const int* pin2node_map;

  const int* flat_net2pin_start_map;
  const int* flat_net2pin_map;
  const int* pin2net_map;

  const int* flat_region_boxes_start;
  const float* flat_region_boxes;
  const int* node2fence_region_map;

  const int* net_mask;  // used for HPWL calculation

  /* row info */
  int numSitesX;
  int numSitesY;
  float rowHeight;
  float siteWidth;

  /* GPU info */
  int numThreads;               
  int numBinsX;                 
  int numBinsY;                 
  float binSizeX;               
  float binSizeY;    

  /* candidates info (single height cells) */
         

private:
  void createChipInfo();
  void createRegionInfo();
  void createNodeInfo();
  void createPinInfo();
  void createGPUInfo();
  void createNetInfo();
};

} // namespace dpo