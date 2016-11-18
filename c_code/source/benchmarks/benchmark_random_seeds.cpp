#include "../include/benchmark_random_seeds.h"

static void PrintBenchmarkRandomSeedsUsage() {
    printf("Usage: benchmarks <flags> - RANDOM SEED SET\n");
    printf("Benchmark flags:\n");
    printf("\t-o [output path/prefix] : Specifies the prefix for output files.\n");
    printf("\t-s [seed file name] : Specifies a file with the seed sets (if not specified, k random seeds will be picked for each simulation).\n");
    printf("\t-k [number of seeds] : Specifies the number of seeds that will be picked at random at each simulation.\n");
    printf("\t-x [number of simulations]: Specifies the number of simulations.\n");
    printf("\t-t [walk length] : Specifies the length of the random walks (default value: 2).\n");
    printf("\t-a [algorithm] : Specifies the algorithm(default value: 0).\n");
    printf("\t\t0: PageRank (with conductance as objective function)\n");
    printf("\t\t2: WalkScan (use flags --epsilon and --min-elems to change parameter values)\n");
}

int RunBenchmarkRandomSeeds(int argc, char ** argv,
                            std::vector <std::pair<uint32_t, uint32_t> >& edgeList,
                            uint32_t maxNodeId,
                            std::vector< NodeSet >& nodeNeighbors,
                            std::vector< NodeSet >& groundTruthCommunities,
                            std::vector< NodeSet >& nodeGroundTruthCommunities) {

    bool outputFileNameSet = false;
    bool seedSetFileNameSet = false;
    bool numSeedsSet = false;
    bool numSimulationsSet = false;
    bool walkLengthSet = false;
    bool algorithmIdSet = false;
    bool epsilonSet = false;
    bool minElemsSet = false;
    char * outputFileName = NULL;
    uint32_t numSeeds = 0;
    uint32_t numSimulations = 0;
    char * seedSetFileName = NULL;
    uint32_t walkLength = 2;
    uint32_t algorithmId = 0;
    double epsilon = 0.01;
    uint32_t minElems = 2;

    for(int i = 1; i < argc; i++) {
        CHECK_ARGUMENT_STRING(i, "-o", outputFileName, outputFileNameSet)
        CHECK_ARGUMENT_STRING(i, "-s", seedSetFileName, seedSetFileNameSet)
        CHECK_ARGUMENT_INT(i, "-k", numSeeds, numSeedsSet)
        CHECK_ARGUMENT_INT(i, "-x", numSimulations, numSimulationsSet)
        CHECK_ARGUMENT_INT(i, "-t", walkLength, walkLengthSet);
        CHECK_ARGUMENT_INT(i, "-a", algorithmId, algorithmIdSet);
        CHECK_ARGUMENT_FLOAT(i, "--epsilon", epsilon, epsilonSet);
        CHECK_ARGUMENT_FLOAT(i, "--min-elems", minElems, minElemsSet);
    }

    if (!outputFileNameSet) {
        printf("Output filename not set\n");
        PrintBenchmarkRandomSeedsUsage();
        return 1;
    }

    if (!seedSetFileNameSet) {
        if (!numSeedsSet || !numSimulationsSet) {
            printf("Number of seeds unspecified\n");
            PrintBenchmarkRandomSeedsUsage();
            return 1;
        }
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
                PrintBenchmarkRandomSeedsUsage();
                return 1;
        }
    } else {
        printf("Algorithm not specified\n");
        PrintBenchmarkRandomSeedsUsage();
        return 1;
    }

    if (walkLengthSet) {
        printf("Length of random walks: %i\n", walkLength);
    } else {
        printf("Length of random walks unspecified. Default value used: %i\n", walkLength);
    }

    //====================== LOAD OR BUILD THE SEED SET ============================
    std::vector< NodeSet > seeds;    // Allocating vector for seeds
    std::vector< NodeSet > nodeSeedSet;    // Allocating vector for seeds
    if (seedSetFileNameSet) {
        printf("Seed sets: %s\n", seedSetFileName);
        LoadCommunity(seedSetFileName, seeds, nodeSeedSet);
        printf("Nb of seed sets: %lu\n", seeds.size());
        numSimulations = seeds.size();
        numSeeds = seeds[0].size();
    } else {
        std::string seedSetOutputFileName(outputFileName);
        seedSetOutputFileName += "-seedsets.txt";
        printf("Picking k random seeds for each simulation...\n");
        PickRandomSeeds(maxNodeId, nodeNeighbors, seeds, numSeeds, numSimulations);
        printf("Nb of seed sets: %lu\n", seeds.size());
        printf("Output file for seed sets: %s\n", seedSetOutputFileName.c_str());
        PrintPartition(seedSetOutputFileName.c_str(), seeds);
    }
    std::vector< NodeSet > seedCommunities (numSimulations);
    //======================================================================

    //=================== ALGORITHM  =======================================
    std::vector< NodeSet > communities(numSimulations);    // Allocating vector for communities
    std::vector< double > f1Scores(numSimulations);    // Allocating vector for f1-scores
    double averageF1Score;
    switch (algorithmId) {
        // PAGERANK
        case 0:
        {
            std::vector <std::vector <std::pair<uint32_t, double> > > pageRankResult;
            printf("Computing PageRank...\n");
            PageRank(nodeNeighbors, seeds, walkLength, 0.85, pageRankResult, maxNodeId);
            printf("Computing Min Conductance communities...\n");
            PageRankMinConductanceNoF1(nodeNeighbors, pageRankResult, seeds, communities);
            printf("Computing F1 score...\n");
            //ComputeF1ScorePerSeed(seeds, groundTruthCommunities, nodeGroundTruthCommunities, communities, f1Scores);
            ComputeF1ScoreSeedUnion(seeds, groundTruthCommunities, nodeGroundTruthCommunities, communities, f1Scores);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);
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
            WalkScan(nodeNeighbors, groundTruthCommunities, seeds, walkLength, walkScanResult, maxNodeId, epsilon, minElems, false);
            printf("Computing Max-F1 communities...\n");
            //WalkScanMaxF1PerSeed(walkScanResult, groundTruthCommunities, nodeGroundTruthCommunities, seeds, f1Scores);
            WalkScanF1SeedUnion(walkScanResult, groundTruthCommunities, nodeGroundTruthCommunities, seeds, f1Scores, 0);
            averageF1Score = GetAverage(f1Scores);
            printf("Average F1 score: %f\n", averageF1Score);
            break;
        }
        default:
            printf("Incorrect algorithm ID.\n");
            PrintBenchmarkRandomSeedsUsage();
            return 1;
    }
    //======================================================================

return 0;
}
