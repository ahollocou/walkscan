#include "../include/benchmark.h"

static void PrintBenchmarkUsage() {
    printf("Usage: walkscan <flags> - CLASSIC BENCHMARK\n");
    printf("Benchmark flags:\n");
    printf("\t-o [output path/prefix] : Specifies the prefix for output files.\n");
    printf("\t-s [seed file name] : Specifies a file with the seed sets (if not specified, seed nodes are chosen at random in each ground-truth community).\n");
    printf("\t-t [walk length] : Specifies the length of the random walks (default value: 2).\n");
    printf("\t-a [algorithm] : Specifies the algorithm (default value: 0).\n");
    printf("\t\t0: PageRank\n");
    printf("\t\t1: LexRank\n");
    printf("\t\t2: WalkScan (use flags --epsilon and --min-elems to change parameter values)\n");
    printf("\t-f [objective function] : Specifies the objective function for the sweep algorithm of PageRank or LexRank (default value: 0).\n");
    printf("\t\t0: Max-F1\n");
    printf("\t\t1: Rank Threshold\n");
    printf("\t\t2: Conductance\n");
}

int RunBenchmark(int argc, char ** argv,
                 std::vector <std::pair<uint32_t, uint32_t> >& edgeList,
                 uint32_t maxNodeId,
                 std::vector< NodeSet >& nodeNeighbors,
                 std::vector< NodeSet >& groundTruthCommunities) {

    bool outputFileNameSet = false;
    bool seedSetFileNameSet = false;
    bool walkLengthSet = false;
    bool algorithmIdSet = false;
    bool objectiveFunctionIdSet = false;
    bool epsilonSet = false;
    bool minElemsSet = false;
    char * outputFileName = NULL;
    char * seedSetFileName = NULL;
    uint32_t walkLength = 2;
    uint32_t algorithmId = 0;
    uint32_t objectiveFunctionId = 0;
    double epsilon = 0.01;
    uint32_t minElems = 2;

    for(int i = 1; i < argc; i++) {
        CHECK_ARGUMENT_STRING(i, "-o", outputFileName, outputFileNameSet)
        CHECK_ARGUMENT_STRING(i, "-s", seedSetFileName, seedSetFileNameSet)
        CHECK_ARGUMENT_INT(i, "-t", walkLength, walkLengthSet);
        CHECK_ARGUMENT_INT(i, "-a", algorithmId, algorithmIdSet);
        CHECK_ARGUMENT_INT(i, "-f", objectiveFunctionId, objectiveFunctionIdSet);
        CHECK_ARGUMENT_FLOAT(i, "--epsilon", epsilon, epsilonSet);
        CHECK_ARGUMENT_FLOAT(i, "--min-elems", minElems, minElemsSet);
    }

    if (!outputFileNameSet) {
        printf("Output filename not set\n");
        PrintBenchmarkUsage();
        return 1;
    }

    if (algorithmIdSet) {
        switch (algorithmId) {
            case 0:
                printf("Selected algorithm: PageRank\n");
                break;
            case 1:
                printf("Selected algorithm: PageRank\n");
                break;
            case 2:
                printf("Selected algorithm: PageRank\n");
                break;
            default:
                printf("Invalid algorithm\n");
                PrintBenchmarkUsage();
                return 1;
        }
    } else {
        printf("Algorithm not specified\n");
        PrintBenchmarkUsage();
        return 1;
    }
    
    if (objectiveFunctionIdSet) {
        switch (objectiveFunctionId) {
            case 0:
                printf("Selected objective function: Max F1 (oracle)\n");
                break;
            case 1:
                printf("Selected objective function: Rank Threshold\n");
                break;
            case 2:
                printf("Selected objective function: Conductance\n");
                break;
            default:
                printf("Invalid objective function\n");
                PrintBenchmarkUsage();
                return 1;
        }
    } else if (algorithmId != 2) {
        printf("Objective function not specified\n");
        PrintBenchmarkUsage();
        return 1;
    }

    if (walkLengthSet) {
        printf("Length of random walks: %i\n", walkLength);
    } else {
        printf("Length of random walks unspecified. Default value used: %i\n", walkLength);
    }

    //====================== LOAD OR BUILD THE SEED SET ============================
    std::vector< NodeSet > seeds;    // Allocating vector for seeds
    std::vector< NodeSet > nodeSeedSet (maxNodeId + 1);    // Allocating vector for seeds
    if (seedSetFileNameSet) {
        printf("Seed sets: %s\n", seedSetFileName);
        LoadCommunity(seedSetFileName, seeds, nodeSeedSet);
        printf("Nb of seed sets: %lu\n", seeds.size());
    } else {
        double proportion = 0.1;
        std::string seedSetOutputFileName(outputFileName);
        seedSetOutputFileName += "-seedsets.txt";
        printf("Picking random seeds in each ground-truth community...\n");
        PickRandomSeedsInGroundTruth(groundTruthCommunities, seeds, proportion);
        printf("Nb of seed sets: %lu\n", seeds.size());
        printf("Output file for seed sets: %s\n", seedSetOutputFileName.c_str());
        PrintPartition(seedSetOutputFileName.c_str(), seeds);
    }
    //======================================================================

    //=================== ALGORITHM  =======================================
    std::vector< NodeSet > communities(groundTruthCommunities.size());    // Allocating vector for communities
    std::vector< double > f1Scores(groundTruthCommunities.size());    // Allocating vector for f1-scores
    double averageF1Score;
    switch (algorithmId) {
        // PAGERANK
        case 0:
        {
            std::vector <std::vector <std::pair<uint32_t, double> > > pageRankResult;
            printf("Computing PageRank...\n");
            PageRank(nodeNeighbors, seeds, walkLength, 0.85, pageRankResult, maxNodeId);
            switch (objectiveFunctionId) {
                case 0:
                    printf("Computing Max-F1 communities...\n");
                    PageRankMaxF1(pageRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    break;
                case 1:
                    printf("Computing Rank Threshold communities...\n");
                    PageRankThresholdFindBest(pageRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    break;
                case 2:
                    printf("Computing Min Conductance communities...\n");
                    PageRankMinConductance(nodeNeighbors, pageRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    break;
                default:
                    printf("Incorrect objective function.\n");
                    PrintBenchmarkUsage();
                    return 1;
            }
            break;
        }
        // LEXRANK
        case 1:
        {
            std::vector <std::vector <std::pair<uint32_t, std::vector< double > > > > lexRankResult;
            printf("Computing LexRank...\n");
            LexRank(nodeNeighbors, seeds, walkLength, lexRankResult, maxNodeId);
            switch (objectiveFunctionId) {
                case 0:
                    printf("Computing Max-F1 communities...\n");
                    LexRankMaxF1(lexRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    break;
                case 2:
                    printf("Computing Min Conductance communities...\n");
                    LexRankMinConductance(nodeNeighbors, lexRankResult, groundTruthCommunities, seeds, communities, f1Scores);
                    averageF1Score = GetAverage(f1Scores);
                    printf("Average F1 score: %f\n", averageF1Score);
                    break;
                default:
                    printf("Incorrect objective function.\n");
                    PrintBenchmarkUsage();
                    return 1;
            }
            break;
        }
        // WALKSCAN
        case 2:
        {
            if (epsilonSet) {
                printf("Value of epsilon (parameter of DBSCAN): %f\n", epsilon);
            } else {
                printf("Value of epsilon (parameter of DBSCAN) unspecified. Default value is used: %f\n", epsilon);
            }
            if (minElemsSet) {
                printf("Value of min elements (parameter of DBSCAN): %i\n", minElems);
            } else {
                printf("Value of min elements (parameter of DBSCAN) unspecified. Default value is used: %i\n", minElems);
            }
            std::vector <std::vector< NodeSet > > walkScanResult;

            printf("Computing WalkScan...\n");
            WalkScan(nodeNeighbors, groundTruthCommunities, seeds, walkLength, walkScanResult, maxNodeId, epsilon, minElems);

            printf("Computing Max-F1 communities...\n");
            WalkScanMaxF1(walkScanResult, groundTruthCommunities, seeds, communities, f1Scores);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);

            printf("Computing Max-F1 among Top 1 communities...\n");
            WalkScanMaxF1(walkScanResult, groundTruthCommunities, seeds, communities, f1Scores, 1);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);

            printf("Computing Max-F1 among Top 2 communities...\n");
            WalkScanMaxF1(walkScanResult, groundTruthCommunities, seeds, communities, f1Scores, 2);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);

            printf("Computing Max-F1 with 2 communities...\n");
            WalkScan2MaxF1(walkScanResult, groundTruthCommunities, seeds, communities, f1Scores);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);

            break;
        }
        default:
            printf("Incorrect algorithm ID.\n");
            PrintBenchmarkUsage();
            return 1;
    }
    //======================================================================


    //======================== PRINT RESULTS ===============================
    std::string communityOutputFileName(outputFileName);
    communityOutputFileName += "-communities.txt";
    std::string scoreOutputFileName(outputFileName);
    scoreOutputFileName += "-f1scores.txt";
    printf("Printing communities in %s...\n", communityOutputFileName.c_str());
    PrintPartition(communityOutputFileName.c_str(), communities);
    printf("Printing scores in %s...\n", scoreOutputFileName.c_str());
    PrintVector(scoreOutputFileName.c_str(), f1Scores);
    //======================================================================

    return 0;
}
