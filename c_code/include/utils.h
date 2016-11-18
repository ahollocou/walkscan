#ifndef WALKSCAN_UTILS_H
#define WALKSCAN_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <set>
#include <utility>
#include <sys/time.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string.h>
#include "../include/types.h"

#define CHECK_ARGUMENT_STRING(index, option,variable,setVariable) \
    if( strcmp(argv[index],option) == 0 ){ \
            setVariable = true; \
            if( (index+1) < argc ) { \
                variable = argv[index+1]; \
            } else { \
                printf( "Invalid options.\n" ); \
                return 1;\
            }\
        }

#define CHECK_ARGUMENT_FLOAT(index, option,variable,setVariable) \
    if( strcmp(argv[index],option) == 0 ){ \
            setVariable = true; \
            if( (index+1) < argc ) { \
                variable = atof(argv[index+1]); \
            } else { \
                printf( "Invalid options.\n" ); \
                return 1;\
            }\
        }

#define CHECK_ARGUMENT_INT(index, option,variable,setVariable) \
    if( strcmp(argv[index],option) == 0 ){ \
            setVariable = true; \
            if( (index+1) < argc ) { \
                variable = atoi(argv[index+1]); \
            } else { \
                printf( "Invalid options.\n" ); \
                return 1;\
            }\
        }

#define CHECK_FLAG(index, option,setVariable) \
    if( strcmp(argv[index],option) == 0 ){ \
            setVariable = true; \
        }

long unsigned StartClock();
long unsigned StopClock(long unsigned initTime);
void DisplayProgress(double progress, uint32_t barWidth);
int LoadGraph(char * graphFileName, std::vector< Edge >& edgeList, uint32_t& maxNodeId);
int BuildNeighborhoods(std::vector< Edge >& edgeList, std::vector< NodeSet >& nodeNeighbors);
int LoadCommunity(char * communityFileName, std::vector< NodeSet >& communities, std::vector< NodeSet >& nodeCommunities);
int PickRandomSeedsInGroundTruth(std::vector< NodeSet >& communities, std::vector< NodeSet >& seeds, double proportion);
int PickRandomSeedsNearGroundTruth(std::vector< NodeSet >& nodeNeighbors, std::vector< NodeSet >& communities, std::vector< NodeSet >& seeds,
                                   double proportion, uint32_t distanceToGroundTruth);
int PickRandomSeeds(uint32_t maxNodeId, std::vector< NodeSet >& nodeNeighbors, std::vector< NodeSet >& seeds,
                    uint32_t numSeeds, uint32_t numSimulations);
int BuildCommunities(uint32_t * nodeCommunity, std::map< uint32_t, NodeSet >& communities, uint32_t maxNodeId);
double GetAverage(std::vector< double >& vector);
int PrintPartition(const char* fileName, std::vector< NodeSet >& communities);
int PrintVector(const char* fileName, std::vector< double >& vector);
void PrintSet(NodeSet set);
void PrintPageRank(std::vector< double > pageRank, NodeSet support);

#endif
