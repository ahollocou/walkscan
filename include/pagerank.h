#ifndef WALKSCAN_PAGERANK_H
#define WALKSCAN_PAGERANK_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>
#include "../include/utils.h"
#include "../include/scores.h"

int PageRank(std::vector< NodeSet >& nodeNeighbors, std::vector< NodeSet >& seeds,
             uint32_t nbSteps, double alpha,
             std::vector< std::vector< NodePageRank > >& pageRankResult,
             uint32_t maxNodeId);
bool nodePageRankCompare(const NodePageRank& node1, const NodePageRank& node2);
int PageRankMaxF1(std::vector< std::vector< NodePageRank > >& pageRankResult,
                  std::vector< NodeSet >& groundTruthCommunities,
                  std::vector< NodeSet >& seeds,
                  std::vector< NodeSet >& communities,
                  std::vector< double >& f1Scores);
int PageRankThreshold(std::vector< std::vector< NodePageRank > >& pageRankResult,
                      std::vector< NodeSet >& groundTruthCommunities,
                      std::vector< NodeSet >& seeds,
                      std::vector< NodeSet >& communities,
                      std::vector< double >& f1Scores,
                      double pageRankThreshold);
int PageRankThresholdFindBest(std::vector <std::vector< NodePageRank > >& pageRankResult,
                              std::vector< NodeSet >& groundTruthCommunities,
                              std::vector< NodeSet >& seeds,
                              std::vector< NodeSet >& bestCommunities,
                              std::vector< double >& bestF1Scores);
int PageRankMinConductance(std::vector< NodeSet >& nodeNeighbors,
                           std::vector< std::vector< NodePageRank > >& pageRankResult,
                           std::vector< NodeSet >& groundTruthCommunities,
                           std::vector< NodeSet >& seeds,
                           std::vector< NodeSet >& communities,
                           std::vector< double >& f1Scores);
int PageRankMinConductanceNoF1(std::vector< NodeSet >& nodeNeighbors,
                               std::vector <std::vector< NodePageRank > >& pageRankResult,
                               std::vector< NodeSet >& seeds,
                               std::vector< NodeSet >& communities);
#endif
