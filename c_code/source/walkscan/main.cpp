#include "../include/utils.h"
#include "../include/benchmark.h"
#include "../include/benchmark_random_seeds.h"
#include "../include/benchmark_locally_random_seeds.h"

static void PrintUsage() {
    printf("Usage: walkscan <flags>\n");
    printf("Availaible flags:\n");
    printf("\t-i [graph file name] : Specifies the graph file (tab-separated list of edges).\n");
    printf("\t-o [output path/prefix] : Specifies the prefix for output files.\n");
    printf("\t-s [seed file name] : Specifies a file with the seed sets.\n");
    printf("\t-t [walk length] : Specifies the length of the random walks (default value: 2).\n");
    printf("\t-a [algorithm] : Specifies the algorithm (default value: 0).\n");
    printf("\t\t0: PageRank with conductance\n");
    printf("\t\t1: LexRank with conductance\n");
    printf("\t\t2: WalkScan (use flags --epsilon and --min-elems to change parameter values)\n");
}

int main(int argc, char ** argv) {
    bool graphFileNameSet = false;
    bool outputFileNameSet = false;
    bool seedSetFileNameSet = false;
    bool walkLengthSet = false;
    bool algorithmIdSet = false;
    bool epsilonSet = false;
    bool minElemsSet = false;
    char * graphFileName = NULL;
    char * outputFileName = NULL;
    char * seedSetFileName = NULL;
    uint32_t walkLength = 2;
    uint32_t algorithmId = 0;
    double epsilon = 0.01;
    uint32_t minElems = 2;

    for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage();
            return 0;
        }
        CHECK_ARGUMENT_STRING(i, "-i", graphFileName, graphFileNameSet);
        CHECK_ARGUMENT_STRING(i, "-o", outputFileName, outputFileNameSet)
        CHECK_ARGUMENT_STRING(i, "-s", seedSetFileName, seedSetFileNameSet)
        CHECK_ARGUMENT_INT(i, "-t", walkLength, walkLengthSet);
        CHECK_ARGUMENT_INT(i, "-a", algorithmId, algorithmIdSet);
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

    if (!seedSetFileNameSet) {
        printf("Seed set file name not set\n");
        PrintUsage();
        return 1;
    }

    if (algorithmIdSet) {
        switch (algorithmId) {
            case 0:
                printf("Selected algorithm: PageRank\n");
                break;
            case 1:
                printf("Selected algorithm: LexRank\n");
                break;
            case 2:
                printf("Selected algorithm: WalkSCAN\n");
                break;
            default:
                printf("Invalid algorithm\n");
                PrintUsage();
                return 1;
        }
    } else {
        printf("Algorithm not specified\n");
        PrintUsage();
        return 1;
    }

    if (walkLengthSet) {
        printf("Length of random walks: %i\n", walkLength);
    } else {
        printf("Length of random walks unspecified. Default value used: %i\n", walkLength);
    }

    //==================== LOAD THE GRAPH ==================================
    std::vector< Edge > edgeList;  // Allocating list for edges
    printf("Graph file: %s\n", graphFileName);
    uint32_t maxNodeId;
    LoadGraph(graphFileName, edgeList, maxNodeId);
    std::vector< NodeSet > nodeNeighbors (maxNodeId + 1);
    BuildNeighborhoods(edgeList, nodeNeighbors);
    printf("Nb of edges: %lu\n", edgeList.size());
    //======================================================================

    //====================== LOAD OR BUILD THE SEED SET ============================
    std::vector< NodeSet > seeds;    // Allocating vector for seeds
    std::vector< NodeSet > nodeSeedSet (maxNodeId + 1);    // Allocating vector for seeds
    printf("Seed sets: %s\n", seedSetFileName);
    LoadCommunity(seedSetFileName, seeds, nodeSeedSet);
    printf("Nb of seed sets: %lu\n", seeds.size());
    //======================================================================

    //=================== ALGORITHM  =======================================
    std::vector< NodeSet > communities(seeds.size());    // Allocating vector for communities
    switch (algorithmId) {
        // PAGERANK
        case 0:
        {
            std::vector <std::vector <std::pair<uint32_t, double> > > pageRankResult;
            printf("Computing PageRank...\n");
            PageRank(nodeNeighbors, seeds, walkLength, 0.85, pageRankResult, maxNodeId);
            printf("Computing Min Conductance communities...\n");
            PageRankMinConductanceNoF1(nodeNeighbors, pageRankResult, seeds, communities);
            break;
        }
        // LEXRANK
        case 1:
        {
            std::vector <std::vector <std::pair<uint32_t, std::vector< double > > > > lexRankResult;
            printf("Computing LexRank...\n");
            LexRank(nodeNeighbors, seeds, walkLength, lexRankResult, maxNodeId);
            LexRankMinConductanceNoF1(nodeNeighbors, lexRankResult, seeds, communities);
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
            WalkScanNoGroundTruth(nodeNeighbors, seeds, walkLength, walkScanResult, maxNodeId, epsilon, minElems);
            WalkScanFirstCommunity(walkScanResult, seeds, communities);

            break;
        }
        default:
            printf("Incorrect algorithm ID.\n");
            PrintUsage();
            return 1;
    }
    //======================================================================

    //======================== PRINT RESULTS ===============================
    std::string communityOutputFileName(outputFileName);
    communityOutputFileName += "-communities.txt";
    printf("Printing communities in %s...\n", communityOutputFileName.c_str());
    PrintPartition(communityOutputFileName.c_str(), communities);
    //======================================================================

    return 0;
}
