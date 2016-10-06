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

void DisplayProgress(double progress, unsigned int barWidth) {
    std::cout << "[";
    unsigned int pos = barWidth * progress;
    for (unsigned int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void PrintPageRank(std::vector <double> pageRank, std::set<unsigned int> support) {
    for (std::set<unsigned int>::iterator it = support.begin();
         it != support.end(); ++it) {
        printf("Node %i -> %f\n", *it, pageRank[*it]);
    }
}

int LoadGraph(char * graphFileName,
              std::vector <std::pair<unsigned int, unsigned int> >& edgeList,
              unsigned int& maxNodeId) {
    // Opening file
    std::ifstream inFile;
    inFile.open((const char *)graphFileName);
    if(!inFile) {
        printf( "Error Openning Graph File\n" );
        return 1;
    }
    // Loading edges
    unsigned int node1, node2;
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

int BuildNeighborhoods(std::vector <std::pair<unsigned int, unsigned int> >& edgeList,
                       std::vector<std::set<unsigned int> >& nodeNeighbors) {
    for (std::vector <std::pair<unsigned int, unsigned int> >::iterator it1 = edgeList.begin();
         it1 != edgeList.end(); ++it1) {
        std::pair<unsigned int, unsigned int> edge(*it1);
        nodeNeighbors[edge.first].insert(edge.second);
        nodeNeighbors[edge.second].insert(edge.first);
    }
    return 0;
}

int LoadCommunity(char * communityFileName,
                  std::vector <std::set<unsigned int> >& communities) {
    // Opening file
    std::ifstream inFile;
    inFile.open((const char *)communityFileName);
    if(!inFile) {
        printf( "Error Openning Community File\n" );
        return 1;
    }
    // Loading communities
    std::string line;
    unsigned int nodeId;
    while(std::getline(inFile, line)) {
        std::stringstream linestream(line);
        std::set<unsigned int> community;
        while(linestream >> nodeId) {
            community.insert(nodeId);
        }
        communities.push_back(community);
    }
    inFile.close();
    return 0;
}

int PickRandomSeeds(std::vector <std::set<unsigned int> >& communities,
                    std::vector <std::set<unsigned int> >& seeds,
                    double proportion) {
    std::srand(std::time(0));
    for (std::vector<std::set<unsigned int> >::iterator it1 = communities.begin();
         it1 != communities.end();
         ++it1) {
        std::vector<unsigned int> community((*it1).begin(), (*it1).end());
        std::random_shuffle(community.begin(), community.end());
        std::set<unsigned int> seedSet;
        unsigned int seedSetSize = ceil(((double) (*it1).size()) * proportion);
        std::vector<unsigned int>::iterator it2 = community.begin();
        unsigned int i = 0;
        while(it2 != community.end() && i < seedSetSize) {
            seedSet.insert((*it2));
            ++it2;
            ++i;
        }
        seeds.push_back(seedSet);
    }
    return 0;
}

int BuildCommunities(unsigned int * nodeCommunity,
                     std::map<unsigned int, std::set<unsigned int> >& communities,
                     unsigned int maxNodeId) {
    for (unsigned int i = 0; i <= maxNodeId; i++) {
        communities[nodeCommunity[i]].insert(i);
    }
    return 0;
}

double GetAverage(std::vector<double>& vector) {
    double sum = 0.0;
    for (std::vector<double>::iterator it = vector.begin(); it != vector.end(); ++it) {
        sum += *it;
    }
    return sum / ((double) vector.size());
}

int PrintPartition(const char* fileName,
                   std::vector<std::set<unsigned int> >& communities) {
    std::ofstream outFile;
    outFile.open(fileName);
    for (std::vector<std::set<unsigned int> >::iterator it1 = communities.begin();
         it1 != communities.end();
         ++it1) {
        std::set<unsigned int>::iterator it2 = (*it1).begin();
        unsigned int nodeId;
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

int PrintVector(const char* fileName,
                std::vector<double>& vector) {
    std::ofstream outFile;
    outFile.open(fileName);
    for (std::vector<double>::iterator it = vector.begin();
         it != vector.end(); ++it) {
        outFile << *it << std::endl;
    }
    outFile.close();
    return 0;
}

unsigned int ComputeIntersectionSize(std::set<unsigned int> community1, std::set<unsigned int> community2) {
    unsigned int intersection = 0;
    if( community1.size() < community2.size() ) {
        for( std::set<unsigned int>::iterator iterCom1 = community1.begin(); iterCom1 != community1.end(); iterCom1++ ) {
            if( community2.find( *iterCom1 ) != community2.end() ) {
                intersection++;
            }
        }
    } else {
        for( std::set<unsigned int>::iterator iterCom2 = community2.begin(); iterCom2 != community2.end(); iterCom2++ ) {
            if( community1.find( *iterCom2 ) != community1.end() ) {
                intersection++;
            }
        }
    }
    return intersection;
}

double ComputeF1Score(std::set<unsigned int> community1, std::set<unsigned int> community2) {
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

double ComputeF1Score(unsigned int sizeCommunity1, unsigned int sizeCommunity2, unsigned int sizeIntersection) {
    double f1Score = 0.0;
    if (sizeIntersection > 0 && sizeCommunity1 > 0 && sizeCommunity2 > 0) {
        double a = ((double) sizeIntersection) / ((double) sizeCommunity1);
        double b = ((double) sizeIntersection) / ((double) sizeCommunity2);
        f1Score = 2.0 * a * b / (a + b);
    }
    return f1Score;
}

double ComputeConductance(unsigned int degreeSum, unsigned int internalEdges) {
    double cut = (degreeSum / 2) - internalEdges;
    double conductance = cut / (((double) degreeSum) + cut);
    return conductance;
}
