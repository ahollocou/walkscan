#include <stdio.h>
#include <cstdlib>
#include <string.h>
#include <sys/time.h>
#include <fstream>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include "../include/utils.h"
#include "../include/pagerank.h"
#include "../include/lexrank.h"
#include "../include/walkscan.h"

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

static void PrintUsage() {
    printf("Usage: walkscan <flags>\n");
    printf("Availaible flags:\n");
    printf("\t-i [graph file name] : Specifies the graph file (tab-separated list of edges).\n");
    printf("\t-o [output path/prefix] : Specifies the prefix for output files.\n");
    printf("\t-s [seed file name] : Specifies a file with the seed sets (if not specified, seeds are chosen at random in ground-truth communities).\n");
    printf("\t-c [community file name] : Specifies a file with ground-truth communities (tab-separated list of nodes).\n");
    printf("\t-t [walk length] : Specifies the length of the random walks (default value: 2).\n");
    printf("\t-a [algorithm]: Specifies the algorithm(default value: 0).\n");
    printf("\t\t0: PageRank\n");
    printf("\t\t1: LexRank\n");
    printf("\t\t2: WalkScan (use flags --epsilon and --min-elems to change parameter values)\n");
    printf("\t-f [stopping criterion]: Specifies the stopping criterion for PageRank or LexRank (default value: 0).\n");
    printf("\t\t0: Max-F1\n");
    printf("\t\t1: Rank Threshold\n");
    printf("\t\t2: Conductance\n");
}

int main(int argc, char ** argv) {
    bool graphFileNameSet = false;
    bool outputFileNameSet = false;
    bool communityFileNameSet = false;
    bool seedSetFileNameSet = false;
    bool walkLengthSet = false;
    bool algorithmIdSet = false;
    bool stoppingCriterionIdSet = false;
    bool epsilonSet = false;
    bool minElemsSet = false;
    char * graphFileName = NULL;
    char * outputFileName = NULL;
    char * communityFileName = NULL;
    char * seedSetFileName = NULL;
    unsigned int walkLength = 2;
    unsigned int algorithmId = 0;
    unsigned int stoppingCriterionId = 0;
    double epsilon = 0.01;
    unsigned int minElems = 2;

    for(int i = 1; i < argc; i++) {
        CHECK_ARGUMENT_STRING(i, "-i", graphFileName, graphFileNameSet);
        CHECK_ARGUMENT_STRING(i, "-o", outputFileName, outputFileNameSet)
        CHECK_ARGUMENT_STRING(i, "-s", seedSetFileName, seedSetFileNameSet)
        CHECK_ARGUMENT_STRING(i, "-c", communityFileName, communityFileNameSet)
        CHECK_ARGUMENT_INT(i, "-t", walkLength, walkLengthSet);
        CHECK_ARGUMENT_INT(i, "-a", algorithmId, algorithmIdSet);
        CHECK_ARGUMENT_INT(i, "-f", stoppingCriterionId, stoppingCriterionIdSet);
        CHECK_ARGUMENT_FLOAT(i, "--epsilon", epsilon, epsilonSet);
        CHECK_ARGUMENT_FLOAT(i, "--min-elems", minElems, minElemsSet);
    }

    if (!graphFileNameSet) {
        printf("Graph filename not set\n");
        PrintUsage();
        return 1;
    }

    if (!outputFileNameSet) {
        printf("Output filename not set\n");
        PrintUsage();
        return 1;
    }

    if (!communityFileNameSet) {
        printf("No ground-truth communities\n");
        PrintUsage();
        return 1;
    }

    long unsigned totalTime = 0,
                  initTime = 0,
                  spentTime = 0,
                  loadingTime = 0,
                  seedingTime = 0,
                  algorithmTime = 0;

    //==================== LOAD THE GRAPH ==================================
    std::vector <std::pair<unsigned int, unsigned int> > edgeList;  // Allocating list for edges
    initTime = StartClock();
    printf("Graph file: %s\n", graphFileName);
    unsigned int maxNodeId;
    LoadGraph(graphFileName, edgeList, maxNodeId);
    std::vector<std::set <unsigned int> > nodeNeighbors (maxNodeId + 1);
    BuildNeighborhoods(edgeList, nodeNeighbors);
    spentTime = StopClock(initTime);
    loadingTime = spentTime;
    totalTime += spentTime;
    printf("Load time: %lu ms\n", spentTime);
    printf("Nb of edges: %lu\n", edgeList.size());
    //======================================================================

    //============= LOAD THE GROUND-TRUTH COMMUNITIES ======================
    std::vector <std::set<unsigned int> > groundTruthCommunities;  // Allocating vector for communities
    initTime = StartClock();
    printf("Ground-truth communities: %s\n", communityFileName);
    LoadCommunity(communityFileName, groundTruthCommunities);
    spentTime = StopClock(initTime);
    loadingTime += spentTime;
    totalTime += spentTime;
    printf("Load time: %lu ms\n", spentTime);
    printf("Nb of communities: %lu\n", groundTruthCommunities.size());
    //======================================================================

    //====================== LOAD OR BUILD THE SEED SET ============================
    std::vector <std::set<unsigned int> > seeds;    // Allocating vector for seeds
    if (seedSetFileNameSet) {
        initTime = StartClock();
        printf("Seed sets: %s\n", seedSetFileName);
        LoadCommunity(seedSetFileName, seeds);
        spentTime = StopClock(initTime);
        seedingTime = spentTime;
        totalTime += spentTime;
        printf("Load time: %lu ms\n", spentTime);
        printf("Nb of seed sets: %lu\n", seeds.size());
    } else {
        double proportion = 0.1;
        std::string seedSetOutputFileName(outputFileName);
        seedSetOutputFileName += "-seedsets.txt";
        initTime = StartClock();
        printf("Picking random seeds in each ground-truth community...\n");
        PickRandomSeeds(groundTruthCommunities, seeds, proportion);
        printf("Nb of seed sets: %lu\n", seeds.size());
        printf("Output file for seed sets: %s\n", seedSetOutputFileName.c_str());
        PrintPartition(seedSetOutputFileName.c_str(), seeds);
        spentTime = StopClock(initTime);
        seedingTime = spentTime;
        totalTime += spentTime;
        printf("Seed set construction time: %lu ms\n", spentTime);
    }
    //======================================================================

    //=================== ALGORITHM  =======================================
    std::vector <std::set<unsigned int> > communities(groundTruthCommunities.size());    // Allocating vector for communities
    std::vector <double> f1Scores(groundTruthCommunities.size());    // Allocating vector for f1-scores
    double averageF1Score;
    std::vector <std::vector <std::pair<unsigned int, double> > > pageRankResult;
    std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > > lexRankResult;
    std::vector <std::vector <std::set <unsigned int> > > walkScanResult;
    std::vector <double> pageRankThresholds(groundTruthCommunities.size());
    double averagePageRankThreshold;
    switch (algorithmId) {
        // PAGERANK
        case 0:
            initTime = StartClock();
            printf("Computing PageRank...\n");
            PageRank(nodeNeighbors, seeds, walkLength, 0.85, pageRankResult, maxNodeId);
            spentTime = StopClock(initTime);
            algorithmTime += spentTime;
            totalTime += spentTime;
            printf("PageRank computing time: %lu ms\n", spentTime);
            switch (stoppingCriterionId) {
                case 0:
                    initTime = StartClock();
                    printf("Computing Max-F1 communities...\n");
                    PageRankMaxF1(pageRankResult, groundTruthCommunities, seeds, communities, f1Scores, pageRankThresholds);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Max-F1 computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    averagePageRankThreshold = GetAverage(pageRankThresholds);
                    printf("Average Page Rank Threshold: %f\n", averagePageRankThreshold);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                case 1:
                    initTime = StartClock();
                    printf("Computing Rank Threshold communities...\n");
                    PageRankThresholdFindBest(pageRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Rank Threshold computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                case 2:
                    initTime = StartClock();
                    printf("Computing Min Conductance communities...\n");
                    PageRankMinConductance(nodeNeighbors, pageRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Min Conductance computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                default:
                    printf("Incorrect stopping criterion.\n");
                    PrintUsage();
                    return 1;
            }
            break;
        // LEXRANK
        case 1:
            initTime = StartClock();
            printf("Computing LexRank...\n");
            LexRank(nodeNeighbors, seeds, walkLength, lexRankResult, maxNodeId);
            spentTime = StopClock(initTime);
            algorithmTime += spentTime;
            totalTime += spentTime;
            printf("LexRank computing time: %lu ms\n", spentTime);
            switch (stoppingCriterionId) {
                case 0:
                    initTime = StartClock();
                    printf("Computing Max-F1 communities...\n");
                    LexRankMaxF1(lexRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Max-F1 computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                case 1:
                    initTime = StartClock();
                    printf("Computing Rank Threshold communities...\n");
                    LexRankThresholdFindBest(lexRankResult, groundTruthCommunities, seeds, communities, f1Scores, walkLength);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Rank Threshold computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                case 2:
                    initTime = StartClock();
                    printf("Computing Min Conductance communities...\n");
                    LexRankMinConductance(nodeNeighbors, lexRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    spentTime = StopClock(initTime);
                    algorithmTime += spentTime;
                    totalTime += spentTime;
                    printf("Min Conductance computing time: %lu ms\n", spentTime);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    printf("Algorithm total time: %lu ms\n", algorithmTime);
                    break;
                default:
                    printf("Incorrect stopping criterion.\n");
                    PrintUsage();
                    return 1;
            }
            break;
        // WALKSCAN
        case 2:
            initTime = StartClock();
            printf("Computing WalkScan...\n");
            WalkScan(nodeNeighbors, groundTruthCommunities, seeds, walkLength, walkScanResult, maxNodeId, epsilon, minElems);
            spentTime = StopClock(initTime);
            algorithmTime += spentTime;
            totalTime += spentTime;
            printf("WalkScan computing time: %lu ms\n", spentTime);
            initTime = StartClock();
            printf("Computing Max-F1 communities...\n");
            WalkScanMaxF1(walkScanResult, groundTruthCommunities, seeds, communities, f1Scores);
            spentTime = StopClock(initTime);
            algorithmTime += spentTime;
            totalTime += spentTime;
            printf("Max-F1 computing time: %lu ms\n", spentTime);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);
            printf("Algorithm total time: %lu ms\n", algorithmTime);
            break;
        default:
            printf("Incorrect algorithm ID.\n");
            PrintUsage();
            return 1;
    }
    //======================================================================

    //======================== PRINT RESULTS ===============================
    initTime = StartClock();
    std::string communityOutputFileName(outputFileName);
    communityOutputFileName += "-communities.txt";
    std::string scoreOutputFileName(outputFileName);
    scoreOutputFileName += "-f1scores.txt";
    printf("Printing communities in %s...\n", communityOutputFileName.c_str());
    PrintPartition(communityOutputFileName.c_str(), communities);
    printf("Printing scores in %s...\n", scoreOutputFileName.c_str());
    PrintVector(scoreOutputFileName.c_str(), f1Scores);
    spentTime = StopClock(initTime);
    totalTime += spentTime;
    printf("Print partition time: %lu ms\n", spentTime);
    //======================================================================

    printf("\n");
    printf("*******************************************************\n");
    printf("%-32s %lu\n", "Number of Communities:", communities.size());
    printf("%-32s %f\n", "Average F1 Score:", averageF1Score);
    printf("%-32s %-10lu ms\n", "Loading time:", loadingTime);
    printf("%-32s %-10lu ms\n", "Seeding time:", seedingTime);
    printf("%-32s %-10lu ms\n", "Algorithm time:", algorithmTime);
    printf("%-32s %-10lu ms\n", "Total execution time:", totalTime);
    printf("*******************************************************\n");

    return 0;
}
