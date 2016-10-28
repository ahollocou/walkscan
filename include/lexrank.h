#ifndef WALKSCAN_LEXRANK_H
#define WALKSCAN_LEXRANK_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>
#include "../include/utils.h"
#include "../include/scores.h"

int LexRank(std::vector< NodeSet >& nodeNeighbors,
            std::vector< NodeSet >& seedSets,
            uint32_t nbSteps,
            std::vector< std::vector< NodeLexRank > >& lexRankResult,
            uint32_t maxNodeId);
bool nodeLexRankCompare(const NodeLexRank& node1, const NodeLexRank& node2);
int LexRankMaxF1(std::vector< std::vector< NodeLexRank > >& lexRankResult,
                 std::vector< NodeSet >& groundTruthCommunities,
                 std::vector< NodeSet >& seedSets,
                 std::vector< NodeSet >& communities,
                 std::vector< double >& f1Scores);
int LexRankMinConductance(std::vector< NodeSet >& nodeNeighbors,
                          std::vector< std::vector< NodeLexRank > >& lexRankResult,
                          std::vector< NodeSet >& groundTruthCommunities,
                          std::vector< NodeSet >& seedSets,
                          std::vector< NodeSet >& communities,
                          std::vector< double >& f1Scores);

#endif
