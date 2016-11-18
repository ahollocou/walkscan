#include "../include/lexrank.h"
#include "../include/utils.h"

int LexRank(std::vector< NodeSet >& nodeNeighbors,
            std::vector< NodeSet >& seedSets,
            uint32_t nbSteps,
            std::vector< std::vector< NodeLexRank > >& lexRankResult,
            uint32_t maxNodeId) {
    uint32_t counter = 0;
    uint32_t nbCommunities = seedSets.size();
    for (std::vector< NodeSet >::iterator it1 = seedSets.begin(); it1 != seedSets.end(); ++it1) {
        DisplayProgress(((double) counter) / (double) nbCommunities, 100);
        NodeSet seedSet(*it1);
        uint32_t seedSetSize = seedSet.size();
        NodeSet walkSupport;
        std::vector< std::vector< double > > walkProba (nbSteps + 1);
        std::vector< bool > isSeed (maxNodeId, false);
        // Initialization of the walk from the seed nodes
        walkProba[0].resize(maxNodeId + 1);
        for (NodeSet::iterator it2 = seedSet.begin(); it2 != seedSet.end(); ++it2) {
                walkProba[0][*it2] = 1.0 / ((double) seedSetSize);
                walkSupport.insert(*it2);
                isSeed[*it2] = true;
        }
        // For each step
        for (uint32_t t = 0; t < nbSteps; t++) {
            NodeSet nextWalkSupport(walkSupport);
            walkProba[t + 1].resize(maxNodeId + 1);
            // For each node with a pagerank > 0 at the previous step
            for (NodeSet::iterator it2 = walkSupport.begin(); it2 != walkSupport.end(); ++it2) {
                uint32_t node1 = *it2;
                NodeSet neighbors = nodeNeighbors[node1];
                double degree = neighbors.size();
                for (NodeSet::iterator it3 = neighbors.begin(); it3 != neighbors.end(); ++it3) {
                    // The walk goes to one of its neighbor with probability 1 / degree
                    uint32_t node2 = *it3;
                    walkProba[t + 1][node2] += walkProba[t][node1] / degree;
                    nextWalkSupport.insert(node2);
                }
            }
            walkSupport = nextWalkSupport;
        }
        // Building output
        std::vector< NodeLexRank > nodeLexRank;
        for (NodeSet::iterator it2 = walkSupport.begin(); it2 != walkSupport.end(); ++it2) {
            uint32_t node = *it2;
            if (!isSeed[node]) {
                std::vector< double > lexRank(nbSteps);
                for (uint32_t t = 0; t < nbSteps; t++) {
                    lexRank[t] = walkProba[t + 1][node];
                }
                nodeLexRank.push_back(std::make_pair(node, lexRank));
            }
        }
        std::sort(nodeLexRank.begin(), nodeLexRank.end(), nodeLexRankCompare);
        lexRankResult.push_back(nodeLexRank);
        counter++;
    }
    std::cout << std::endl;
    return 0;
}

bool nodeLexRankCompare(const NodeLexRank& node1, const NodeLexRank& node2) {
    return node1.second > node2.second;
}

int LexRankMaxF1(std::vector< std::vector< NodeLexRank > >& lexRankResult,
                 std::vector< NodeSet >& groundTruthCommunities,
                 std::vector< NodeSet >& seedSets,
                 std::vector< NodeSet >& communities,
                 std::vector< double >& f1Scores) {
    uint32_t nbCommunities = groundTruthCommunities.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        NodeSet groundTruthCommunity(groundTruthCommunities[i]);
        uint32_t groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector< NodeLexRank > nodeLexRank(lexRankResult[i]);
        NodeSet community(seedSets[i]);
        uint32_t communitySize = community.size();
        NodeSet bestCommunity(community);
        uint32_t intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity);
        double bestF1Score = ComputeF1Score(communitySize, groundTruthCommunitySize, intersectionSize);
        for (std::vector< NodeLexRank >::iterator it = nodeLexRank.begin(); it != nodeLexRank.end(); ++it) {
            uint32_t node = (*it).first;
            community.insert(node);
            communitySize++;
            if (groundTruthCommunity.find(node) != groundTruthCommunity.end()) {
                intersectionSize ++;
            }
            double f1Score = ComputeF1Score(communitySize, groundTruthCommunitySize, intersectionSize);
            if (f1Score >= bestF1Score) {
                bestCommunity = community;
                bestF1Score = f1Score;
            } else if (communitySize > 2 * groundTruthCommunitySize) {
                break;
            }
        }
        communities[i] = bestCommunity;
        f1Scores[i] = bestF1Score;
    }
    std::cout << std::endl;
    return 0;
}

int LexRankMinConductanceNoF1(std::vector< NodeSet >& nodeNeighbors,
                              std::vector< std::vector< NodeLexRank > >& lexRankResult,
                              std::vector< NodeSet >& seedSets,
                              std::vector< NodeSet >& communities) {
    uint32_t nbCommunities = seedSets.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::vector< NodeLexRank > nodeLexRank(lexRankResult[i]);
        NodeSet community(seedSets[i]);
        uint32_t communitySize = community.size();
        NodeSet bestCommunity(community);
        uint32_t internalEdges = 0;
        uint32_t degreeSum = 0;
        for (NodeSet::iterator it = community.begin(); it != community.end(); ++it) {
            uint32_t node = *it;
            NodeSet neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
        }
        double bestConductance = ComputeConductance(degreeSum, internalEdges);

        for (std::vector< NodeLexRank >::iterator it = nodeLexRank.begin();
             it != nodeLexRank.end(); ++it) {
            uint32_t node = (*it).first;
            community.insert(node);
            communitySize++;
            NodeSet neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
            double conductance = ComputeConductance(degreeSum, internalEdges);
            if (conductance <= bestConductance) {
                bestCommunity = community;
                bestConductance = conductance;
            }
        }
        communities[i] = bestCommunity;
    }
    std::cout << std::endl;
    return 0;
}


int LexRankMinConductance(std::vector< NodeSet >& nodeNeighbors,
                          std::vector< std::vector< NodeLexRank > >& lexRankResult,
                          std::vector< NodeSet >& groundTruthCommunities,
                          std::vector< NodeSet >& seedSets,
                          std::vector< NodeSet >& communities,
                          std::vector< double >& f1Scores) {
    uint32_t nbCommunities = groundTruthCommunities.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        NodeSet groundTruthCommunity(groundTruthCommunities[i]);
        uint32_t groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector< NodeLexRank > nodeLexRank(lexRankResult[i]);
        NodeSet community(seedSets[i]);
        uint32_t communitySize = community.size();
        NodeSet bestCommunity(community);
        uint32_t internalEdges = 0;
        uint32_t degreeSum = 0;
        for (NodeSet::iterator it = community.begin(); it != community.end(); ++it) {
            uint32_t node = *it;
            NodeSet neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
        }
        double bestConductance = ComputeConductance(degreeSum, internalEdges);


        for (std::vector< NodeLexRank >::iterator it = nodeLexRank.begin();
             it != nodeLexRank.end(); ++it) {
            uint32_t node = (*it).first;
            community.insert(node);
            communitySize++;
            NodeSet neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
            double conductance = ComputeConductance(degreeSum, internalEdges);
            if (conductance <= bestConductance) {
                bestCommunity = community;
                bestConductance = conductance;
            } else if (communitySize > 2 * groundTruthCommunitySize) {
                break;
            }
        }
        communities[i] = bestCommunity;
        f1Scores[i] = ComputeF1Score(bestCommunity, groundTruthCommunity);
    }
    std::cout << std::endl;
    return 0;
}
