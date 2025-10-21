#include <torch/extension.h>

#include <vector>
#include <iostream>

#include "architecture.h"
#include "detailed.h"
#include "detailed_db.h"
#include "detailed_manager.h"
#include "legalize_shift.h"
#include "network.h"
#include "orientation.h"
#include "router.h"
#include "symmetry.h"

namespace dpo2 {

DetailedPlaceDB::DetailedPlaceDB(Network* network, Architecture* arch, DetailedMgr* mgr) {
  network_ = network;
  arch_ = arch;
  mgr_ = mgr;
}

void DetailedPlaceDB::createDetailedPlaceDB() {
  createChipInfo();
  setupIndexMap();  
  createNodeInfo(); 
  createRegionInfo();
  createNode2PinInfo();
  createNet2PinInfo();
  createRowInfo();
  createGPUInfo();
}

void DetailedPlaceDB::createChipInfo() {
  numNodes = network_->getNumNodes();
  numPins = network_->getNumPins();
  numEdges = network_->getNumEdges();
  numRegions = arch_->getNumRegions();
  numMovableNodes = mgr_->getSingleHeightCells().size();
  xl = arch_->getMinX();
  yl = arch_->getMinY();
  xh = arch_->getMaxX();
  yh = arch_->getMaxY();
}

void DetailedPlaceDB::createRowInfo() {
  
}

void DetailedPlaceDB::createGPUInfo(int num_threads, int num_bins_x, int num_bins_y) {
  numThreads = num_threads;
  numBinsX = num_bins_x;
  numBinsY = num_bins_y;
  binSizeX = (xh - xl) / numBinsX;
  binSizeY = (yh - yl) / numBinsY;
}

void DetailedPlaceDB::setupIndexMap() {
  // used to create 1:1 mappings between important components (nodes, pins, nets, etc.)
  std::vector<int> pin_id2node_id_vec;
  std::vector<int> pin_id2net_id_vec;
  for (int i = 0; i < network_->getNumNodes(); i++) {
    Node* node = network->getNode(i);
    for (int j = 0; j < node->getPins().size(); j++) {
      Pin* pin = node->getPins()[j];
      pin_id2node_id.emplace_back(node->getId());
    }
  }
  for (int i = 0; i < network_->getNumEdges(); i++) {
    Edge* edge = network_->getEdge(i);
    for (int j = 0; j < edge->getPins().size(); j++) {
      Pin* pin = edge->getPins()[j];
      pin_id2net_id.emplace_back(j);
    }
  }
  pin_id2net_id = pin_id2net_id_vec.data();
  pin_id2node_id = pin_id2node_id_vec.data();
}

void DetailedPlaceDB::createRegionInfo() {
  auto optionsInt = torch::TensorOptions().dtype(torch::kInt64);

  unsigned numRects = 0;
  for (auto& region : arch_->getRegions()) {
    numRects += region->getRects().size();
  }
  
  torch::Tensor node_id2region_id = torch::zeros({numNodes}, optionsInt);
  torch::Tensor region_boxes = torch::zeros({numRects, 4});
  torch::Tensor region_boxes_end = torch::zeros({numRegions}, optionsInt);

  auto node_id2region_id_accessor = node_id2region_id.accessor<int64_t, 1>();
  auto region_boxes_accessor = region_boxes.accessor<float, 2>();
  auto region_boxes_end_accessor = region_boxes_end.accessor<int64_t, 1>();

  for (int i = 0; i < network_->getNumNodes(); i++) {
    Node* nd = network_->getNode(i);
    int node_id = nd->getId();
    int region_id = nd->getRegionId();
    node_id2region_id_accessor[node_id] = region_id;
  }

  int ptr = 0;
  int lastIdx = 0;
  for (auto& region : arch_->getRegions()) {
    int region_id = region->getId();
    for (auto& rect : region->getRects()) {
      region_boxes_accessor[ptr][0] = rect.getMinX();
      region_boxes_accessor[ptr][1] = rect.getMinY();
      region_boxes_accessor[ptr][2] = rect.getMaxX();
      region_boxes_accessor[ptr][3] = rect.getMaxY();
      ptr++;
    }
    lastIdx += region->getRects().size();
    region_boxes_end_accessor[region_id] = lastIdx;
  }
  node2fence_region_map = node_id2region_id.data_ptr<int>();
  flat_region_boxes = region_boxes.flatten().contiguous().clone().data_ptr<float>();
  flat_region_boxes_start = torch::cat({torch::zeros({1}, torch::dtype(torch::kInt32).device(torch::Device(nodeSizeTensor.device()))),
                    region_boxes_end},
                   0)
            .contiguous().data_ptr<int>();
}

void DetailedPlaceDB::createNodeInfo() {
  // store the node coordinates
  torch::Tensor node_coord = torch::zeros({numNodes, 2});
  auto node_coord_accesor = node_coord.accessor<float, 2>();
  for (int i = 0; i < network_->getNumNodes(); i++) {
    Node* node = network_->getNode(i);
    int node_id = node->getId();
    node_coord_accesor[node_id][0] = node->getLeft();
    node_coord_accesor[node_id][1] = node->getBottom();
  }

  // store the node centroids
  // torch::Tensor node_centroid = torch::zeros({numNodes, 2});
  // auto node_centroid_accessor = node_centroid.accessor<float, 2>();
  // for (int i = 0; i < network_->getNumNodes(); i++) {
  //   Node* node = network_->getNode(i);
  //   int node_id = node->getId();
  //   node_centroid_accessor[node_id][0] = node->getLeft() + 0.5 * node->getWidth();
  //   node_centroid_accessor[node_id][1] = node->getBottom() + 0.5 * node->getHeight();
  // }

  // store the node sizes
  torch::Tensor node_size = torch::zeros({numNodes, 2});
  auto node_size_accessor = node_size.accessor<float, 2>();
  for (int i = 0; i < network_->getNumNodes(); i++) {
    Node* node = network_->getNode(i);
    int node_id = node->getId();
    node_size_accessor[node_id][0] = node->getWidth();
    node_size_accessor[node_id][1] = node->getHeight();
  }
  initX = node_coord.index({"...", 0}).clone().contiguous().data_ptr<float>();
  initY = node_coord.index({"...", 1}).clone().contiguous().data_ptr<float>();
  nodeSizeX = node_size.index({"...", 0}).clone().contiguous().data_ptr<float>();
  nodeSizeY = node_size.index({"...", 1}).clone().contiguous().data_ptr<float>();
  x = node_coord.index({"...", 0}).clone().contiguous().data_ptr<float>();
  y = node_coord.index({"...", 1}).clone().contiguous().data_ptr<float>();
  nodeSizeTensor = node_size;
}

void DetailedPlaceDB::createNode2PinInfo() {
  auto options = torch::TensorOptions().dtype(torch::kInt64);

  //torch::Tensor node2pin_index_helper = torch::zeros({numPins}, options);
  torch::Tensor node2pin_list = torch::zeros({numPins}, options);
  torch::Tensor node2pin_list_end = torch::zeros({numNodes}, options);
  torch::Tensor node2pin_offsetx_list = torch::zeros({numPins}, options);
  torch::Tensor node2pin_offsety_list = torch::zeros({numPins}, options);
  torch::Tensor node2pin_width_list = torch::zeros({numPins}, options);
  torch::Tensor node2pin_height_list = torch::zeros({numPins}, options); 

  //auto node2pin_index_helper_accessor = node2pin_index_helper.accessor<int64_t, 1>();
  auto node2pin_list_accessor = node2pin_list.accessor<int64_t, 1>();
  auto node2pin_list_end_accessor = node2pin_list_end.accessor<int64_t, 1>();
  auto node2pin_offsetx_list_accessor = node2pin_offsetx_list.accessor<int64_t, 1>();
  auto node2pin_offsety_list_accessor = node2pin_offsety_list.accessor<int64_t, 1>();
  auto node2pin_width_list_accessor = node2pin_width_list.accessor<int64_t, 1>();
  auto node2pin_height_list_accessor = node2pin_height_list.accessor<int64_t, 1>();

  int ptr = 0;
  int lastIdx = 0;
  for (int i = 0; i < network_->getNumNodes(); i++) {
    Node* node = network_->getNode(i);
    int node_id = node->getId();
    for (auto& pin : node->getPins()) {
      node2pin_list_accessor[ptr] = node_id;
      node2pin_offsetx_list_accessor[ptr] = pin->getOffsetX();
      node2pin_offsety_list_accessor[ptr] = pin->getOffsetY();
      node2pin_width_list_accessor[ptr] = pin->getPinWidth();
      node2pin_height_list_accessor[ptr] = pin->getPinHeight();
      ptr++;
    }
    lastIdx += node->getPins().size();
    node2pin_list_end_accessor[node_id] = lastIdx;
  }

  // sort all the pin information based on increasing node id
  auto node2pin_index = torch::cat({
    node2pin_list.unsqueeze(0),
    node2pin_offsetx_list.unsqueeze(0),
    node2pin_offsety_list.unsqueeze(0),
    node2pin_width_list.unsqueeze(0),
    node2pin_height_list.unsqueeze(0)
  }, 0);

  auto new_order_idx = torch::argsort(node2pin_index.index({0}), 0, false);
  node2pin_index = node2pin_index.index({torch::indexing::Slice(), new_order_idx});
  //auto node2pin_index = torch::cat({node2pin_list.unsqueeze(0), node2pin_offsetx_list_accessor.unsqueeze(0)}, 0);
  //auto new_order_idx = torch::argsort(node2pin_index.index({0}), 0, false);
  //node2pin_index = node2pin_index.index({torch::indexing::Slice(), new_order_idx});
  flat_node2pin_map = node2pin_list.data_ptr<int>();
  flat_node2pin_start_map = torch::cat({torch::zeros({1}, torch::dtype(torch::kInt32).device(torch::Device(nodeSizeTensor.device()))),
                    node2pin_list_end},
                   0)
            .contiguous().data_ptr<int>();
}

void DetailedPlaceDB::createNet2PinInfo() {
  auto options = torch::TensorOptions().dtype(torch::kInt64);

  //torch::Tensor net_index_helper = torch::zeros({numPins}, options);
  torch::Tensor net_list = torch::zeros({numPins}, options);
  torch::Tensor net_list_end = torch::zeros({numEdges}, options);

  //auto net_index_helper_accessor = net_index_helper.accessor<int64_t, 1>();
  auto net_list_accessor = net_list.accessor<int64_t, 1>();
  auto net_list_end_accessor = net_list_end.accessor<int64_t, 1>();

  int ptr = 0;
  int lastIdx = 0;
  for (int i = 0; i < network_->getNumEdges(); i++) {
    Edge* edge = network_->getEdge(i);
    int edge_id = edge->getId();
    for (auto& pin : edge->getPins()) {
      net_list_accessor[ptr] = edge_id;
      ptr++;
    }
    lastIdx += edge->getPins().size();
    net_list_end_accessor[edge_id] = lastIdx;
  }
  auto net_index = torch::cat({net_list.unsqueeze(0)}, 0);
  auto new_order_idx = torch::argsort(net_index.index({0}), 0, false);
  net_index = net_index.index({torch::indexing::Slice(), new_order_idx});

  std::vector<int> edgeMaskTemp;
  edgeMaskTemp.resize(network_->getNumEdges());
  std::fill(edgeMaskTemp.begin(), edgeMaskTemp.end(), 0);
  edgeMask = edgeMaskTemp.data();

  flat_net2pin_map = net_list.data_ptr<int>();
  flat_net2pin_start_map = torch::cat({torch::zeros({1}, torch::dtype(torch::kInt32).device(torch::Device(nodeSizeTensor.device()))),
                    net_list_end},
                   0)
            .contiguous().data_ptr<int>();
}

void DetailedPlaceDB::getCandidates(std::vector<Node*>& candidates) {
  candidates = mgr_->getSingleHeightCells();

  // flatten single height cells
  
}

} // namespace dpo2