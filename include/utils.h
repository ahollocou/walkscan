#ifndef DYCOLA_UTILS_H
#define DYCOLA_UTILS_H

#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <utility>
#include <sys/time.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <ctime>

long unsigned StartClock();
long unsigned StopClock(long unsigned initTime);
void DisplayProgress(double progress, unsigned int barWidth);
void PrintPageRank(std::vector <double> pageRank, std::set<unsigned int> support);
int LoadGraph(char * graphFileName,
              std::vector <std::pair<unsigned int, unsigned int> >& edgeList,
              unsigned int& maxNodeId);
int BuildNeighborhoods(std::vector <std::pair<unsigned int, unsigned int> >& edgeList,
                       std::vector<std::set<unsigned int> >& nodeNeighbors);
int LoadCommunity(char * communityFileName,
                  std::vector <std::set<unsigned int> >& communities);
int PickRandomSeeds(std::vector <std::set<unsigned int> >& communities,
                    std::vector <std::set<unsigned int> >& seeds,
                    double proportion);
int BuildCommunities(unsigned int * nodeCommunity,
                     std::map<unsigned int, std::set<unsigned int> >& communities,
                     unsigned int maxNodeId);
double GetAverage(std::vector<double>& vector);
int PrintPartition(const char* fileName,
                   std::vector<std::set<unsigned int> >& communities);
int PrintVector(const char* fileName,
                std::vector<double>& vector);
unsigned int ComputeIntersectionSize(std::set<unsigned int> community1, std::set<unsigned int> community2);
double ComputeF1Score(std::set<unsigned int> community1, std::set<unsigned int> community2);
double ComputeF1Score(unsigned int sizeCommunity1, unsigned int sizeCommunity2, unsigned int sizeIntersection);
double ComputeConductance(unsigned int degreeSum, unsigned int internalEdges);

#endif
