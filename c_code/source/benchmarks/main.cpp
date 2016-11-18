#include "../include/utils.h"
#include "../include/benchmark.h"
#include "../include/benchmark_random_seeds.h"
#include "../include/benchmark_locally_random_seeds.h"

static void PrintUsage() {
    printf("Usage: benchmarks <flags>\n");
    printf("Availaible flags:\n");
    printf("\t-i [graph file name] : Specifies the graph file (tab-separated list of edges).\n");
    printf("\t-c [community file name] : Specifies the file with the ground-truth communities (tab-separated list of nodes).\n");
    printf("\t-b [benchmark type] :\n");
    printf("\t\t0 -> Classic benchmark: we pick a seed set in each ground-truth community.\n");
    printf("\t\t1 -> Random seed set: we pick k random seed nodes in the graph (and we make x simulations).\n");
    printf("\t\t2 -> Locally random seed set: we pick one seed set in the neighborhood of each ground-truth community.\n");
}

int main(int argc, char ** argv) {
    bool graphFileNameSet = false;
    bool communityFileNameSet = false;
    bool benchmarkTypeSet = false;
    char * graphFileName = NULL;
    char * communityFileName = NULL;
    uint32_t benchmarkType = 0;

    for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage();
            return 0;
        }
        CHECK_ARGUMENT_STRING(i, "-i", graphFileName, graphFileNameSet);
        CHECK_ARGUMENT_STRING(i, "-c", communityFileName, communityFileNameSet)
        CHECK_ARGUMENT_INT(i, "-b", benchmarkType, benchmarkTypeSet)
    }

    if (!graphFileNameSet) {
        printf("Graph filename not set\n");
        PrintUsage();
        return 1;
    }

    if (!communityFileNameSet) {
        printf("No ground-truth communities\n");
        PrintUsage();
        return 1;
    }

    if (!benchmarkTypeSet) {
        printf("No benchmark type specified\n");
        PrintUsage();
        return 1;
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

    //============= LOAD THE GROUND-TRUTH COMMUNITIES ======================
    std::vector< NodeSet > groundTruthCommunities;  // Allocating vector for communities
    std::vector< NodeSet > nodeGroundTruthCommunities(maxNodeId + 1);
    printf("Ground-truth communities: %s\n", communityFileName);
    LoadCommunity(communityFileName, groundTruthCommunities, nodeGroundTruthCommunities);
    printf("Nb of communities: %lu\n", groundTruthCommunities.size());
    //======================================================================

    //========================= RUN BENCHMARK ==============================
    switch (benchmarkType) {
        case 0:
            RunBenchmark(argc, argv, edgeList, maxNodeId, nodeNeighbors, groundTruthCommunities);
            break;
        case 1:
            RunBenchmarkRandomSeeds(argc, argv, edgeList, maxNodeId, nodeNeighbors, groundTruthCommunities, nodeGroundTruthCommunities);
            break;
        case 2:
            RunBenchmarkLocallyRandomSeeds(argc, argv, edgeList, maxNodeId, nodeNeighbors, groundTruthCommunities);

            break;
        default:
            printf("Incorrect benchark type.");
            PrintUsage();
    }
    //======================================================================

    return 0;
}
