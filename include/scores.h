#ifndef WALKSCAN_SCORES_H
#define WALKSCAN_SCORES_H

#include "../include/utils.h"

uint32_t ComputeIntersectionSize(NodeSet community1, NodeSet community2);
double ComputeF1Score(NodeSet community1, NodeSet community2);
double ComputeF1Score(uint32_t sizeCommunity1, uint32_t sizeCommunity2, uint32_t sizeIntersection);
double ComputeConductance(uint32_t degreeSum, uint32_t internalEdges);
int ComputeF1ScorePerSeed(std::vector< NodeSet >& seedSets,
                          std::vector< NodeSet >& groundTruthCommunities,
                          std::vector< NodeSet >& nodeGroundTruthCommunities,
                          std::vector< NodeSet >& communities,
                          std::vector< double >& f1Scores);
int ComputeF1ScoreSeedUnion(std::vector< NodeSet >& seedSets,
                            std::vector< NodeSet >& groundTruthCommunities,
                            std::vector< NodeSet >& nodeGroundTruthCommunities,
                            std::vector< NodeSet >& communities,
                            std::vector< double >& f1Scores);
#endif
