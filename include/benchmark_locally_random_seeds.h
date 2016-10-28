#ifndef WALKSCAN_BENCHMARK_LOCALLY_RANDOM_SEEDS_H
#define WALKSCAN_BENCHMARK_LOCALLY_RANDOM_SEEDS_H

#include "../include/utils.h"
#include "../include/scores.h"
#include "../include/pagerank.h"
#include "../include/lexrank.h"
#include "../include/walkscan.h"

int RunBenchmarkLocallyRandomSeeds(int argc, char ** argv,
                                   std::vector <std::pair<uint32_t, uint32_t> >& edgeList,
                                   uint32_t maxNodeId,
                                   std::vector <NodeSet >& nodeNeighbors,
                                   std::vector <NodeSet >& groundTruthCommunities);

#endif
