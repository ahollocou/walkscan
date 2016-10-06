#include "../include/walkscan.h"
#include "../include/lexrank.h"
#include "../include/utils.h"

int WalkScan(std::vector<std::set<unsigned int> >& nodeNeighbors,
             std::vector <std::set<unsigned int> >& groundTruthCommunities,
             std::vector <std::set <unsigned int> >& seeds,
             unsigned int nbSteps,
             std::vector <std::vector <std::set<unsigned int> > > & walkScanResult,
             unsigned int maxNodeId,
             double epsilon,
             unsigned int minElems) {
    unsigned int counter = 0;
    unsigned int nbCommunities = seeds.size();
    for (std::vector <std::set <unsigned int> >::iterator it1 = seeds.begin();
         it1 != seeds.end(); ++it1) {
        DisplayProgress(((double) counter) / (double) nbCommunities, 100);
        std::set <unsigned int> seedSet(*it1);
        unsigned int seedSetSize = seedSet.size();
        std::set <unsigned int> walkSupport;
        std::vector <std::vector <double> > walkProba (nbSteps + 1);
        std::vector <bool> isSeed (maxNodeId, false);
        // Initialization of the walk from the seed nodes
        walkProba[0].resize(maxNodeId + 1);
        for (std::set <unsigned int>::iterator it2 = seedSet.begin();
             it2 != seedSet.end(); ++it2) {
                walkProba[0][*it2] = 1.0 / ((double) seedSetSize);
                walkSupport.insert(*it2);
                isSeed[*it2] = true;
        }
        // For each step
        for (unsigned int t = 0; t < nbSteps; t++) {
            std::set <unsigned int> nextWalkSupport(walkSupport);
            walkProba[t + 1].resize(maxNodeId + 1);
            // For each node with a pagerank > 0 at the previous step
            for (std::set <unsigned int>::iterator it2 = walkSupport.begin();
                 it2 != walkSupport.end(); ++it2) {
                unsigned int node1 = *it2;
                std::set <unsigned int> neighbors = nodeNeighbors[node1];
                double degree = neighbors.size();
                for (std::set <unsigned int>::iterator it3 = neighbors.begin();
                     it3 != neighbors.end(); ++it3) {
                    // The walker goes to one of its neighbor with probability 1 / degree
                    unsigned int node2 = *it3;
                    walkProba[t + 1][node2] += walkProba[t][node1] / degree;
                    nextWalkSupport.insert(node2);
                }
            }
            walkSupport = nextWalkSupport;
        }
        // Building output
        std::vector<std::pair<unsigned int, std::vector <double> > > nodeProba;
        for (std::set <unsigned int>::iterator it2 = walkSupport.begin();
             it2 != walkSupport.end(); ++it2) {
            unsigned int node = *it2;
            if (!isSeed[node]) {
                std::vector <double> proba(nbSteps);
                for (unsigned int t = 0; t < nbSteps; t++) {
                    proba[t] = walkProba[t + 1][node];
                }
                nodeProba.push_back(std::make_pair(node, proba));
            }
        }
        std::sort(nodeProba.begin(), nodeProba.end(), nodeLexRankCompare);
        std::set <unsigned int> groundTruthCommunity (groundTruthCommunities[counter]);
        unsigned int groundTruthCommunitySize = groundTruthCommunity.size();
        unsigned int walkSupportSize = walkSupport.size();
        unsigned int nbNodes = walkSupportSize - seedSetSize;
        unsigned int sizeLimit = 2 * groundTruthCommunitySize;
        if (nbNodes > sizeLimit) {
            nbNodes = sizeLimit;
        }
        std::vector <unsigned int> nodeList(nbNodes);
        ublas::matrix <double, ublas::row_major, ublas::unbounded_array<double> > nodeEmbedding(nbNodes, nbSteps);
        int nodeIndex = 0;
        for (std::vector<std::pair<unsigned int, std::vector <double> > >::iterator it2 = nodeProba.begin();
             it2 != nodeProba.end(); ++it2) {
            unsigned int node = (*it2).first;
            if (nodeIndex >= nbNodes) {
                break;
            } else if (!isSeed[node]) {
                for (unsigned int t = 0; t < nbSteps; t++) {
                    nodeEmbedding (nodeIndex, t) = (*it2).second[t];
                }
                nodeList[nodeIndex] = node;
                nodeIndex++;
            }
        }
        clustering::DBSCAN dbs (epsilon, minElems, 1);
        dbs.fit(nodeEmbedding);
        clustering::DBSCAN::Labels labels = dbs.get_labels();

        std::vector <std::set <unsigned int> > walkScanSets;
        std::vector <int> nodeSet (maxNodeId + 1, -1);
        std::set <unsigned int> outliers;
        unsigned int nbSets = 0;
        nodeIndex = 0;
        for (std::vector<int32_t>::iterator it2 = labels.begin(); it2 != labels.end(); it2++) {
            if (*it2 < 0) {
                outliers.insert(nodeList[nodeIndex]);
            } else if (*it2 >= nbSets) {
                walkScanSets.resize(*it2 + 1);
                nbSets = *it2 + 1;
                unsigned int node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            } else {
                unsigned int node = nodeList[nodeIndex];
                walkScanSets[*it2].insert(node);
                nodeSet[node] = *it2;
            }
            nodeIndex++;
        }
        for (std::set <unsigned int>::iterator it2 = outliers.begin();
             it2 != outliers.end(); it2++) {
            unsigned int node = *it2;
            std::set <unsigned int> neighborhood = nodeNeighbors[node];
            for (std::set <unsigned int>::iterator it3 = neighborhood.begin();
                 it3 != neighborhood.end(); ++it3) {
                if (nodeSet[*it3] >= 0) {
                    walkScanSets[nodeSet[*it3]].insert(node);
                }
            }
        }
        walkScanResult.push_back(walkScanSets);
        counter++;
    }
    std::cout << std::endl;
    return 0;
}

int WalkScanMaxF1(std::vector <std::vector <std::set<unsigned int> > > & walkScanResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        unsigned int groundTruthCommunitySize (groundTruthCommunity.size());
        std::vector <std::set<unsigned int> > walkScanSets(walkScanResult[i]);
        std::set<unsigned int> seedSet(seeds[i]);
        unsigned int seedSetSize = seedSet.size();
        std::set<unsigned int> bestCommunity(seedSet);
        double bestF1Score = ComputeF1Score(seedSetSize, groundTruthCommunitySize, seedSetSize);
        for (std::vector <std::set <unsigned int> >::iterator it = walkScanSets.begin();
             it != walkScanSets.end(); ++it) {
            std::set <unsigned int> community = (*it); 
            unsigned int intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity) + seedSetSize; 
            double f1Score = ComputeF1Score(community.size() + seedSetSize, groundTruthCommunitySize, intersectionSize);
            if (f1Score > bestF1Score) {
                bestF1Score = f1Score;
                bestCommunity = community;
                bestCommunity.insert(seedSet.begin(), seedSet.end());
            }
        }
    communities[i] = bestCommunity;
    f1Scores[i] = bestF1Score;
    }
    std::cout << std::endl;
    return 0;
}
