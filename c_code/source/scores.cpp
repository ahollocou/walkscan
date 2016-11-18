#include "../include/scores.h"

uint32_t ComputeIntersectionSize(NodeSet community1, NodeSet community2) {
    uint32_t intersection = 0;
    if( community1.size() < community2.size() ) {
        for( NodeSet::iterator iterCom1 = community1.begin(); iterCom1 != community1.end(); iterCom1++ ) {
            if( community2.find( *iterCom1 ) != community2.end() ) {
                intersection++;
            }
        }
    } else {
        for( NodeSet::iterator iterCom2 = community2.begin(); iterCom2 != community2.end(); iterCom2++ ) {
            if( community1.find( *iterCom2 ) != community1.end() ) {
                intersection++;
            }
        }
    }
    return intersection;
}

double ComputeF1Score(NodeSet community1, NodeSet community2) {
    unsigned sizeIntersection = ComputeIntersectionSize(community1, community2);
    double sizecommunity1 = community1.size();
    double sizecommunity2 = community2.size();
    double f1Score = 0.0;
    if (sizeIntersection > 0 && sizecommunity1 > 0 && sizecommunity2 > 0) {
        double a = sizeIntersection / sizecommunity1;
        double b = sizeIntersection / sizecommunity2;
        f1Score = 2.0 * a * b / (a + b);
    }
    return f1Score;
}

double ComputeF1Score(uint32_t sizeCommunity1, uint32_t sizeCommunity2, uint32_t sizeIntersection) {
    double f1Score = 0.0;
    if (sizeIntersection > 0 && sizeCommunity1 > 0 && sizeCommunity2 > 0) {
        double a = ((double) sizeIntersection) / ((double) sizeCommunity1);
        double b = ((double) sizeIntersection) / ((double) sizeCommunity2);
        f1Score = 2.0 * a * b / (a + b);
    }
    return f1Score;
}

double ComputeConductance(uint32_t degreeSum, uint32_t internalEdges) {
    double cut = (degreeSum / 2) - internalEdges;
    double conductance = cut / (((double) degreeSum) + cut);
    return conductance;
}

int ComputeF1ScorePerSeed(std::vector< NodeSet >& seedSets,
                          std::vector< NodeSet >& groundTruthCommunities,
                          std::vector< NodeSet >& nodeGroundTruthCommunities,
                          std::vector< NodeSet >& communities,
                          std::vector< double >& f1Scores) {
    uint32_t index = 0;
    for (std::vector< NodeSet >::iterator it1 = seedSets.begin(); it1 != seedSets.end(); ++it1) {
        NodeSet seedSet = (*it1);
        NodeSet community = communities[index];
        double sumF1Score = 0.0;
        for (NodeSet::iterator it2 = seedSet.begin(); it2 != seedSet.end(); ++it2) {
            uint32_t seedNode = (*it2);
            NodeSet seedNodeGroundTruthCommunities = nodeGroundTruthCommunities[seedNode];
            double bestF1Score = 0.0;
            for (NodeSet::iterator it3 = seedNodeGroundTruthCommunities.begin(); it3 != seedNodeGroundTruthCommunities.end(); ++it3) {
                NodeSet groundTruthCommunity = groundTruthCommunities[*it3];
                double f1Score = ComputeF1Score(community, groundTruthCommunity);
                if (f1Score > bestF1Score) {
                    bestF1Score = f1Score;
                }
            }
            sumF1Score += bestF1Score;
        }
        f1Scores[index] = sumF1Score / ((double) seedSet.size());
        index++;
    }
    return 0;
}

int ComputeF1ScoreSeedUnion(std::vector< NodeSet >& seedSets,
                            std::vector< NodeSet >& groundTruthCommunities,
                            std::vector< NodeSet >& nodeGroundTruthCommunities,
                            std::vector< NodeSet >& communities,
                            std::vector< double >& f1Scores) {
    uint32_t index = 0;
    for (std::vector< NodeSet >::iterator it1 = seedSets.begin(); it1 != seedSets.end(); ++it1) {
        NodeSet seedSet = (*it1);
        NodeSet community = communities[index];
        NodeSet targetCommunity;
        for (NodeSet::iterator it2 = seedSet.begin(); it2 != seedSet.end(); ++it2) {
            uint32_t seedNode = (*it2);
            NodeSet seedNodeGroundTruthCommunities = nodeGroundTruthCommunities[seedNode];
            for (NodeSet::iterator it3 = seedNodeGroundTruthCommunities.begin();
                 it3 != seedNodeGroundTruthCommunities.end(); ++it3) {
                NodeSet groundTruthCommunity = groundTruthCommunities[*it3];
                for (NodeSet::iterator it4 = groundTruthCommunity.begin();
                    it4 != groundTruthCommunity.end(); it4++) {
                    targetCommunity.insert(*it4);
                }
            }
        }
        f1Scores[index] = ComputeF1Score(targetCommunity, community);
        index++;
    }
    return 0;
}
