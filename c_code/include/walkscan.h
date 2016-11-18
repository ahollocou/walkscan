#ifndef WALKSCAN_WALKSCAN_H
#define WALKSCAN_WALKSCAN_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <dbscan.h>
#include "../include/utils.h"
#include "../include/scores.h"

int WalkScan(std::vector< NodeSet >& nodeNeighbors,
             std::vector< NodeSet >& groundTruthCommunities,
             std::vector< NodeSet >& seeds,
             uint32_t nbSteps,
             std::vector< std::vector< NodeSet > > & walkScanResult,
             uint32_t maxNodeId,
             double epsilon,
             uint32_t minElems,
             bool useSizeLimit = true);
bool WalkScanCenterCompare(const NodeSetLexRank cluster1, const NodeSetLexRank cluster2);
int WalkScanMaxF1(std::vector< std::vector< NodeSet > > & walkScanResult,
                  std::vector< NodeSet >& groundTruthCommunities,
                  std::vector< NodeSet >& seeds,
                  std::vector< NodeSet >& communities,
                  std::vector< double >& f1Scores,
                  uint32_t expertLimit = 0);
int WalkScan2MaxF1(std::vector< std::vector< NodeSet > > & walkScanResult,
                   std::vector< NodeSet >& groundTruthCommunities,
                   std::vector< NodeSet >& seeds,
                   std::vector< NodeSet >& communities,
                   std::vector< double >& f1Scores,
                   uint32_t expertLimit = 0);
int WalkScanF1SeedUnion(std::vector <std::vector< NodeSet > > & walkScanResult,
                        std::vector< NodeSet >& groundTruthCommunities,
                        std::vector< NodeSet >& nodeGroundTruthCommunities,
                        std::vector< NodeSet >& seeds,
                        std::vector< double >& f1Scores,
                        uint32_t expertLimit);
int WalkScanNoGroundTruth(std::vector< NodeSet >& nodeNeighbors,
                          std::vector< NodeSet >& seeds,
                          uint32_t nbSteps,
                          std::vector< std::vector< NodeSet > > & walkScanResult,
                          uint32_t maxNodeId,
                          double epsilon,
                          uint32_t minElems);
int WalkScanFirstCommunity(std::vector< std::vector< NodeSet > > & walkScanResult,
                           std::vector< NodeSet >& seeds,
                           std::vector< NodeSet >& communities);

#endif
