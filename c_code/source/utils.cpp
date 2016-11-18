#include "../include/utils.h"

long unsigned StartClock() {
    timeval time;
    gettimeofday(&time, NULL);
    long unsigned initTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    return initTime;
}

long unsigned StopClock(long unsigned initTime) {
    timeval time;
    gettimeofday(&time, NULL);
    long unsigned endTime = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    return endTime - initTime;
}

void DisplayProgress(double progress, uint32_t barWidth) {
    std::cout << "[";
    uint32_t pos = barWidth * progress;
    for (uint32_t i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

int LoadGraph(char * graphFileName, std::vector< Edge >& edgeList, uint32_t& maxNodeId) {
    // Opening file
    std::ifstream inFile;
    inFile.open((const char *) graphFileName);
    if(!inFile) {
        printf( "Error Openning Graph File\n" );
        return 1;
    }
    // Loading edges
    uint32_t node1, node2;
    maxNodeId = 0;
    while( inFile >> node1 ) {
        inFile >> node2;
        edgeList.push_back(std::make_pair(node1, node2));
        if (node1 > maxNodeId) {
            maxNodeId = node1;
        }
        if (node2 > maxNodeId) {
            maxNodeId = node2;
        }
    }
    inFile.close();
    return 0;
}

int BuildNeighborhoods(std::vector< Edge >& edgeList, std::vector< NodeSet >& nodeNeighbors) {
    for (std::vector< Edge >::iterator it = edgeList.begin(); it != edgeList.end(); ++it) {
        Edge edge(*it);
        nodeNeighbors[edge.first].insert(edge.second);
        nodeNeighbors[edge.second].insert(edge.first);
    }
    return 0;
}

int LoadCommunity(char * communityFileName, std::vector< NodeSet >& communities, std::vector< NodeSet >& nodeCommunities) {
    // Opening file
    std::ifstream inFile;
    inFile.open((const char *)communityFileName);
    if(!inFile) {
        printf( "Error Openning Community File\n" );
        return 1;
    }
    // Loading communities
    std::string line;
    uint32_t nodeId;
    uint32_t communityIndex = 0;
    while(std::getline(inFile, line)) {
        std::stringstream linestream(line);
        NodeSet community;
        while(linestream >> nodeId) {
            community.insert(nodeId);
            nodeCommunities[nodeId].insert(communityIndex);
        }
        communities.push_back(community);
        communityIndex++;
    }
    inFile.close();
    return 0;
}

int PickRandomSeedsInGroundTruth(std::vector< NodeSet >& communities, std::vector< NodeSet >& seeds, double proportion) {
    std::srand(std::time(0));
    for (std::vector< NodeSet >::iterator it1 = communities.begin(); it1 != communities.end(); ++it1) {
        std::vector< uint32_t > community((*it1).begin(), (*it1).end());
        std::random_shuffle(community.begin(), community.end());
        NodeSet seedSet;
        uint32_t seedSetSize = ceil(((double) (*it1).size()) * proportion);
        std::vector< uint32_t >::iterator it2 = community.begin();
        uint32_t i = 0;
        while(it2 != community.end() && i < seedSetSize) {
            seedSet.insert((*it2));
            ++it2;
            ++i;
        }
        seeds.push_back(seedSet);
    }
    return 0;
}

int PickRandomSeedsNearGroundTruth(std::vector< NodeSet >& nodeNeighbors, std::vector< NodeSet >& communities, std::vector< NodeSet >& seeds,
                                   double proportion, uint32_t distanceToGroundTruth) {
    std::srand(std::time(0));
    for (std::vector< NodeSet >::iterator it1 = communities.begin(); it1 != communities.end(); ++it1) {
        std::vector< uint32_t > community((*it1).begin(), (*it1).end());
        NodeSet seedSet;
        uint32_t seedSetSize = ceil(((double) (*it1).size()) * proportion);
        // We compute the neighborhood
        NodeSet neighborhood;
        for (std::vector< uint32_t >::iterator it2 = community.begin(); it2 != community.end(); ++it2) {
                neighborhood.insert(*it2);
        }
        for (uint32_t t = 0; t < distanceToGroundTruth; t++) {
            NodeSet nextNeighborhood(neighborhood);
            for (NodeSet::iterator it2 = neighborhood.begin();
                 it2 != neighborhood.end(); ++it2) {
                uint32_t node1 = *it2;
                NodeSet neighbors = nodeNeighbors[node1];
                for (NodeSet::iterator it3 = neighbors.begin();
                     it3 != neighbors.end(); ++it3) {
                    uint32_t node2 = *it3;
                    nextNeighborhood.insert(node2);
                }
            }
            neighborhood = nextNeighborhood;
        }
        // We choose random seeds in the neighborhood
        std::vector<  uint32_t > candidates (neighborhood.begin(), neighborhood.end());
        std::random_shuffle(candidates.begin(), candidates.end());
        std::vector< uint32_t >::iterator it2 = candidates.begin();
        uint32_t i = 0;
        while(it2 != candidates.end() && i < seedSetSize) {
            seedSet.insert((*it2));
            ++it2;
            ++i;
        }
        seeds.push_back(seedSet);
    }
    return 0;
}

int PickRandomSeeds(uint32_t maxNodeId, std::vector< NodeSet >& nodeNeighbors, std::vector< NodeSet >& seeds,
                    uint32_t numSeeds, uint32_t numSimulations) {
    std::srand(std::time(0));
    for (uint32_t i = 0; i < numSimulations; i++) {
        NodeSet seedSet;
        for (uint32_t j = 0; j < numSeeds; j++) {
            uint32_t selectedNode = 0;
            while (true) {
                selectedNode = std::rand() % (int)(maxNodeId + 1);
                if (nodeNeighbors[selectedNode].size() > 0) {
                    break;
                }
            }
            seedSet.insert(selectedNode);
        }
        seeds.push_back(seedSet);
    }
    return 0;
}

int BuildCommunities(uint32_t * nodeCommunity, std::map< uint32_t, NodeSet >& communities, uint32_t maxNodeId) {
    for (uint32_t i = 0; i <= maxNodeId; i++) {
        communities[nodeCommunity[i]].insert(i);
    }
    return 0;
}

double GetAverage(std::vector< double >& vector) {
    double sum = 0.0;
    for (std::vector< double >::iterator it = vector.begin(); it != vector.end(); ++it) {
        sum += *it;
    }
    return sum / ((double) vector.size());
}

int PrintPartition(const char* fileName, std::vector< NodeSet >& communities) {
    std::ofstream outFile;
    outFile.open(fileName);
    for (std::vector< NodeSet >::iterator it1 = communities.begin(); it1 != communities.end(); ++it1) {
        NodeSet::iterator it2 = (*it1).begin();
        uint32_t nodeId;
        while ( true ) {
            nodeId = *it2;
            ++it2;
            if (it2 != (*it1).end()) {
                outFile << nodeId << " ";
            } else {
                break;
            }
        }
        outFile << nodeId << std::endl;
    }
    outFile.close();
    return 0;
}

int PrintVector(const char* fileName, std::vector< double >& vector) {
    std::ofstream outFile;
    outFile.open(fileName);
    for (std::vector< double >::iterator it = vector.begin();
         it != vector.end(); ++it) {
        outFile << *it << std::endl;
    }
    outFile.close();
    return 0;
}

void PrintSet(NodeSet set) {
    for (NodeSet::iterator it = set.begin(); it != set.end(); ++it) {
        printf("%i ", *it);
    }
    printf("\n");
}

void PrintPageRank(std::vector< double > pageRank, NodeSet support) {
    for (NodeSet::iterator it = support.begin(); it != support.end(); ++it) {
        printf("Node %i -> %f\n", *it, pageRank[*it]);
    }
}
