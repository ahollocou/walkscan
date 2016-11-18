#include "../include/walkscan.h"
#include "../include/lexrank.h"
#include "../include/utils.h"

int WalkScan(std::vector< NodeSet >& nodeNeighbors,
             std::vector< NodeSet >& groundTruthCommunities,
             std::vector< NodeSet >& seeds,
             uint32_t nbSteps,
             std::vector< std::vector< NodeSet > > & walkScanResult,
             uint32_t maxNodeId,
             double epsilon,
             uint32_t minElems,
             bool useSizeLimit) {
    uint32_t counter = 0;
    uint32_t nbCommunities = seeds.size();
    for (std::vector< NodeSet >::iterator it1 = seeds.begin(); it1 != seeds.end(); ++it1) {
        DisplayProgress(((double) counter) / (double) nbCommunities, 100);
        NodeSet seedSet(*it1);
        uint32_t seedSetSize = seedSet.size();
        NodeSet walkSupport;
        std::vector <std::vector< double > > walkProba (nbSteps + 1);
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
                for (NodeSet::iterator it3 = neighbors.begin();
                     it3 != neighbors.end(); ++it3) {
                    // The walker goes to one of its neighbor with probability 1 / degree
                    uint32_t node2 = *it3;
                    walkProba[t + 1][node2] += walkProba[t][node1] / degree;
                    nextWalkSupport.insert(node2);
                }
            }
            walkSupport = nextWalkSupport;
        }
        // Building output
        std::vector< std::pair< uint32_t, std::vector< double > > > nodeProba;
        for (NodeSet::iterator it2 = walkSupport.begin(); it2 != walkSupport.end(); ++it2) {
            uint32_t node = *it2;
            if (!isSeed[node]) {
                std::vector< double > proba(nbSteps);
                for (uint32_t t = 0; t < nbSteps; t++) {
                    proba[t] = walkProba[t + 1][node];
                }
                nodeProba.push_back(std::make_pair(node, proba));
            }
        }
        std::sort(nodeProba.begin(), nodeProba.end(), nodeLexRankCompare);
        NodeSet groundTruthCommunity (groundTruthCommunities[counter]);
        uint32_t groundTruthCommunitySize = groundTruthCommunity.size();
        uint32_t walkSupportSize = walkSupport.size();
        uint32_t nbNodes = walkSupportSize - seedSetSize;
        uint32_t sizeLimit = 2 * groundTruthCommunitySize;
        if (useSizeLimit && nbNodes > sizeLimit) {
            nbNodes = sizeLimit;
        }
        std::vector< uint32_t > nodeList(nbNodes);
        ublas::matrix <double, ublas::row_major, ublas::unbounded_array< double > > nodeEmbedding(nbNodes, nbSteps);
        uint32_t nodeIndex = 0;
        for (std::vector<std::pair<uint32_t, std::vector< double > > >::iterator it2 = nodeProba.begin();
             it2 != nodeProba.end(); ++it2) {
            uint32_t node = (*it2).first;
            if (nodeIndex >= nbNodes) {
                break;
            } else if (!isSeed[node]) {
                for (uint32_t t = 0; t < nbSteps; t++) {
                    nodeEmbedding (nodeIndex, t) = (*it2).second[t];
                }
                nodeList[nodeIndex] = node;
                nodeIndex++;
            }
        }
        clustering::DBSCAN dbs (epsilon, minElems, 1);
        dbs.fit(nodeEmbedding);
        clustering::DBSCAN::Labels labels = dbs.get_labels();
        std::vector< NodeSet > walkScanSets;
        std::vector< int > nodeSet (maxNodeId + 1, -1);
        NodeSet outliers;
        int32_t nbSets = 0;
        nodeIndex = 0;
        for (std::vector<int32_t>::iterator it2 = labels.begin(); it2 != labels.end(); it2++) {
            if (*it2 < 0) {
                outliers.insert(nodeList[nodeIndex]);
            } else if (*it2 >= nbSets) {
                walkScanSets.resize(*it2 + 1);
                nbSets = *it2 + 1;
                uint32_t node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            } else {
                uint32_t node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            }
            nodeIndex++;
        }
        for (NodeSet::iterator it2 = outliers.begin();
             it2 != outliers.end(); it2++) {
            uint32_t node = *it2;
            NodeSet neighborhood = nodeNeighbors[node];
            for (NodeSet::iterator it3 = neighborhood.begin();
                 it3 != neighborhood.end(); ++it3) {
                if (nodeSet[*it3] >= 0) {
                    walkScanSets[nodeSet[*it3]].insert(node);
                }
            }
        }
        std::vector <NodeSetLexRank > walkScanSetCenters;
        for (std::vector< NodeSet >::iterator it2 = walkScanSets.begin(); it2 != walkScanSets.end(); it2++) {
            NodeSet cluster = (*it2);
            double clusterSize = cluster.size();
            std::vector< double > center (nbSteps);
            for (NodeSet::iterator it3 = cluster.begin();
                 it3 != cluster.end(); it3++) {
                uint32_t node = (*it3);
                for (uint32_t t = 0; t < nbSteps; t++) {
                    center[t] += walkProba[t + 1][node] / clusterSize;
                }
            }
            walkScanSetCenters.push_back(std::make_pair(cluster, center));
        }
        std::sort(walkScanSetCenters.begin(), walkScanSetCenters.end(), WalkScanCenterCompare);
        std::vector< NodeSet > orderedWalkScanSets;
        for (std::vector <NodeSetLexRank >::iterator it2 = walkScanSetCenters.begin();
             it2 != walkScanSetCenters.end(); it2++ ) {
            orderedWalkScanSets.push_back((*it2).first);
        }
        walkScanResult.push_back(orderedWalkScanSets);
        counter++;
    }
    return 0;
}

bool WalkScanCenterCompare(const NodeSetLexRank cluster1, const NodeSetLexRank cluster2) {
    return cluster1.second > cluster2.second;
}

int WalkScanMaxF1(std::vector< std::vector< NodeSet > > & walkScanResult,
                  std::vector< NodeSet >& groundTruthCommunities,
                  std::vector< NodeSet >& seeds,
                  std::vector< NodeSet >& communities,
                  std::vector< double >& f1Scores,
                  uint32_t expertLimit) {
    uint32_t nbCommunities = groundTruthCommunities.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        NodeSet groundTruthCommunity(groundTruthCommunities[i]);
        uint32_t groundTruthCommunitySize (groundTruthCommunity.size());
        std::vector< NodeSet > walkScanSets(walkScanResult[i]);
        NodeSet seedSet(seeds[i]);
        uint32_t seedSetSize = seedSet.size();
        NodeSet bestCommunity(seedSet);
        double bestF1Score = ComputeF1Score(seedSetSize, groundTruthCommunitySize, seedSetSize);
        uint32_t rank = 0;
        for (std::vector< NodeSet >::iterator it = walkScanSets.begin();
             it != walkScanSets.end(); ++it) {
            if (expertLimit > 0 && rank >= expertLimit) {
                break;
            }
            NodeSet community = (*it);
            uint32_t intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity) + seedSetSize;
            double f1Score = ComputeF1Score(community.size() + seedSetSize, groundTruthCommunitySize, intersectionSize);
            if (f1Score > bestF1Score) {
                bestF1Score = f1Score;
                bestCommunity = community;
                bestCommunity.insert(seedSet.begin(), seedSet.end());
            }
            rank++;
        }
        communities[i] = bestCommunity;
        f1Scores[i] = bestF1Score;
    }
    std::cout << std::endl;
    return 0;
}

int WalkScan2MaxF1(std::vector< std::vector< NodeSet > > & walkScanResult,
                   std::vector< NodeSet >& groundTruthCommunities,
                   std::vector< NodeSet >& seeds,
                   std::vector< NodeSet >& communities,
                   std::vector< double >& f1Scores,
                   uint32_t expertLimit) {
    uint32_t nbCommunities = groundTruthCommunities.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        NodeSet groundTruthCommunity(groundTruthCommunities[i]);
        uint32_t groundTruthCommunitySize (groundTruthCommunity.size());
        std::vector< NodeSet > walkScanSets(walkScanResult[i]);
        NodeSet seedSet(seeds[i]);
        uint32_t seedSetSize = seedSet.size();
        NodeSet bestCommunity(seedSet);
        double bestF1Score = ComputeF1Score(seedSetSize, groundTruthCommunitySize, seedSetSize);
        uint32_t rank = 0;
        for (std::vector< NodeSet >::iterator it = walkScanSets.begin();
             it != walkScanSets.end(); ++it) {
            if (expertLimit > 0 && rank >= expertLimit) {
                break;
            }
            NodeSet community = (*it); 
            uint32_t intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity) + seedSetSize; 
            double f1Score = ComputeF1Score(community.size() + seedSetSize, groundTruthCommunitySize, intersectionSize);
            if (f1Score > bestF1Score) {
                bestF1Score = f1Score;
                bestCommunity = community;
                bestCommunity.insert(seedSet.begin(), seedSet.end());
            }
            rank++;
        }
        double bestF1Score2 = bestF1Score;
        NodeSet bestCommunity2(bestCommunity);
        rank = 0;
        for (std::vector< NodeSet >::iterator it = walkScanSets.begin();
                it != walkScanSets.end(); ++it) {
            if (expertLimit > 0 && rank >= expertLimit) {
                break;
            }
            NodeSet community;
            std::set_union((*it).begin(), (*it).end(), bestCommunity.begin(), bestCommunity.end(), std::inserter(community, community.begin()));
            uint32_t intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity);
            double f1Score = ComputeF1Score(community.size(), groundTruthCommunitySize, intersectionSize);
            if (f1Score > bestF1Score2) {
                bestF1Score2 = f1Score;
                bestCommunity2 = community;
            }
            rank++;
        }
        communities[i] = bestCommunity2;
        f1Scores[i] = bestF1Score2;
    }
    std::cout << std::endl;
    return 0;
}

int WalkScanF1SeedUnion(std::vector <std::vector< NodeSet > > & walkScanResult,
                        std::vector< NodeSet >& groundTruthCommunities,
                        std::vector< NodeSet >& nodeGroundTruthCommunities,
                        std::vector< NodeSet >& seeds,
                        std::vector< double >& f1Scores,
                        uint32_t expertLimit) {
    uint32_t nbCommunities = seeds.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::vector< NodeSet > walkScanSets(walkScanResult[i]);
        NodeSet seedSet(seeds[i]);
        NodeSet targetCommunity;
        for (NodeSet::iterator it1 = seedSet.begin();
             it1 != seedSet.end(); ++it1) {
            uint32_t seedNode = (*it1);
            NodeSet seedGroundTruthCommunities = nodeGroundTruthCommunities[seedNode];
            for (NodeSet::iterator it2 = seedGroundTruthCommunities.begin();
                    it2 != seedGroundTruthCommunities.end(); ++it2) {
                NodeSet groundTruthCommunity (groundTruthCommunities[*it2]);
                for (NodeSet::iterator it3 = groundTruthCommunity.begin();
                     it3 != groundTruthCommunity.end(); it3++) {
                    targetCommunity.insert(*it3);
                }
            }
        }
        NodeSet community;
        uint32_t rank = 0;
        for (std::vector< NodeSet >::iterator it1 = walkScanSets.begin();
             it1 != walkScanSets.end(); ++it1) {
            if (expertLimit > 0 && rank >= expertLimit) {
                break;
            }
            NodeSet cluster = *it1;
            for (NodeSet::iterator it2 = cluster.begin();
                 it2 != cluster.end(); it2++) {
                community.insert(*it2);
            }
            rank++;
        }
        f1Scores[i] = ComputeF1Score(community, targetCommunity);
    }
    std::cout << std::endl;
    return 0;
}

int WalkScanNoGroundTruth(std::vector< NodeSet >& nodeNeighbors,
                          std::vector< NodeSet >& seeds,
                          uint32_t nbSteps,
                          std::vector< std::vector< NodeSet > > & walkScanResult,
                          uint32_t maxNodeId,
                          double epsilon,
                          uint32_t minElems) {
    uint32_t counter = 0;
    uint32_t nbCommunities = seeds.size();
    for (std::vector< NodeSet >::iterator it1 = seeds.begin(); it1 != seeds.end(); ++it1) {
        DisplayProgress(((double) counter) / (double) nbCommunities, 100);
        NodeSet seedSet(*it1);
        uint32_t seedSetSize = seedSet.size();
        NodeSet walkSupport;
        std::vector <std::vector< double > > walkProba (nbSteps + 1);
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
                for (NodeSet::iterator it3 = neighbors.begin();
                     it3 != neighbors.end(); ++it3) {
                    // The walker goes to one of its neighbor with probability 1 / degree
                    uint32_t node2 = *it3;
                    walkProba[t + 1][node2] += walkProba[t][node1] / degree;
                    nextWalkSupport.insert(node2);
                }
            }
            walkSupport = nextWalkSupport;
        }
        // Building output
        std::vector< std::pair< uint32_t, std::vector< double > > > nodeProba;
        for (NodeSet::iterator it2 = walkSupport.begin(); it2 != walkSupport.end(); ++it2) {
            uint32_t node = *it2;
            if (!isSeed[node]) {
                std::vector< double > proba(nbSteps);
                for (uint32_t t = 0; t < nbSteps; t++) {
                    proba[t] = walkProba[t + 1][node];
                }
                nodeProba.push_back(std::make_pair(node, proba));
            }
        }
        std::sort(nodeProba.begin(), nodeProba.end(), nodeLexRankCompare);
        uint32_t walkSupportSize = walkSupport.size();
        uint32_t nbNodes = walkSupportSize - seedSetSize;
        std::vector< uint32_t > nodeList(nbNodes);
        ublas::matrix <double, ublas::row_major, ublas::unbounded_array< double > > nodeEmbedding(nbNodes, nbSteps);
        uint32_t nodeIndex = 0;
        for (std::vector<std::pair<uint32_t, std::vector< double > > >::iterator it2 = nodeProba.begin();
             it2 != nodeProba.end(); ++it2) {
            uint32_t node = (*it2).first;
            if (nodeIndex >= nbNodes) {
                break;
            } else if (!isSeed[node]) {
                for (uint32_t t = 0; t < nbSteps; t++) {
                    nodeEmbedding (nodeIndex, t) = (*it2).second[t];
                }
                nodeList[nodeIndex] = node;
                nodeIndex++;
            }
        }
        clustering::DBSCAN dbs (epsilon, minElems, 1);
        dbs.fit(nodeEmbedding);
        clustering::DBSCAN::Labels labels = dbs.get_labels();
        std::vector< NodeSet > walkScanSets;
        std::vector< int > nodeSet (maxNodeId + 1, -1);
        NodeSet outliers;
        int32_t nbSets = 0;
        nodeIndex = 0;
        for (std::vector<int32_t>::iterator it2 = labels.begin(); it2 != labels.end(); it2++) {
            if (*it2 < 0) {
                outliers.insert(nodeList[nodeIndex]);
            } else if (*it2 >= nbSets) {
                walkScanSets.resize(*it2 + 1);
                nbSets = *it2 + 1;
                uint32_t node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            } else {
                uint32_t node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            }
            nodeIndex++;
        }
        for (NodeSet::iterator it2 = outliers.begin();
             it2 != outliers.end(); it2++) {
            uint32_t node = *it2;
            NodeSet neighborhood = nodeNeighbors[node];
            for (NodeSet::iterator it3 = neighborhood.begin();
                 it3 != neighborhood.end(); ++it3) {
                if (nodeSet[*it3] >= 0) {
                    walkScanSets[nodeSet[*it3]].insert(node);
                }
            }
        }
        std::vector <NodeSetLexRank > walkScanSetCenters;
        for (std::vector< NodeSet >::iterator it2 = walkScanSets.begin(); it2 != walkScanSets.end(); it2++) {
            NodeSet cluster = (*it2);
            double clusterSize = cluster.size();
            std::vector< double > center (nbSteps);
            for (NodeSet::iterator it3 = cluster.begin();
                 it3 != cluster.end(); it3++) {
                uint32_t node = (*it3);
                for (uint32_t t = 0; t < nbSteps; t++) {
                    center[t] += walkProba[t + 1][node] / clusterSize;
                }
            }
            walkScanSetCenters.push_back(std::make_pair(cluster, center));
        }
        std::sort(walkScanSetCenters.begin(), walkScanSetCenters.end(), WalkScanCenterCompare);
        std::vector< NodeSet > orderedWalkScanSets;
        for (std::vector <NodeSetLexRank >::iterator it2 = walkScanSetCenters.begin();
             it2 != walkScanSetCenters.end(); it2++ ) {
            orderedWalkScanSets.push_back((*it2).first);
        }
        walkScanResult.push_back(orderedWalkScanSets);
        counter++;
    }
    return 0;
}

int WalkScanFirstCommunity(std::vector< std::vector< NodeSet > > & walkScanResult,
                           std::vector< NodeSet >& seeds,
                           std::vector< NodeSet >& communities) {
    uint32_t nbCommunities = seeds.size();
    for (uint32_t i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::vector< NodeSet > walkScanSets(walkScanResult[i]);
        NodeSet seedSet(seeds[i]);
        NodeSet community;
        if (walkScanSets.size() > 0) {
            community = walkScanSets[0];
        }
        community.insert(seedSet.begin(), seedSet.end());
        communities[i] = community;
    }
    std::cout << std::endl;
    return 0;
}
