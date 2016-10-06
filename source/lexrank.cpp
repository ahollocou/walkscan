#include "../include/lexrank.h"
#include "../include/utils.h"

int LexRank(std::vector<std::set<unsigned int> >& nodeNeighbors,
            std::vector <std::set <unsigned int> >& seeds,
            unsigned int nbSteps,
            std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
            unsigned int maxNodeId) {
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
        std::vector<std::pair<unsigned int, std::vector <double> > > nodeLexRank;
        for (std::set <unsigned int>::iterator it2 = walkSupport.begin();
             it2 != walkSupport.end(); ++it2) {
            unsigned int node = *it2;
            if (!isSeed[node]) {
                std::vector <double> lexRank(nbSteps);
                for (unsigned int t = 0; t < nbSteps; t++) {
                    lexRank[t] = walkProba[t + 1][node];
                }
                nodeLexRank.push_back(std::make_pair(node, lexRank));
            }
        }
        std::sort(nodeLexRank.begin(), nodeLexRank.end(), nodeLexRankCompare);
        lexRankResult.push_back(nodeLexRank);
        counter++;
    }
    std::cout << std::endl;
    return 0;
}

bool nodeLexRankCompare(const std::pair<unsigned int, std::vector <double> >& node1, const std::pair<unsigned int, std::vector <double> >& node2) {
    return node1.second > node2.second;
}

int LexRankMaxF1(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        unsigned int groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector<std::pair<unsigned int, std::vector <double> > > nodeLexRank(lexRankResult[i]);
        std::set<unsigned int> community(seeds[i]);
        unsigned int communitySize = community.size();
        std::set<unsigned int> bestCommunity(community);
        unsigned int intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity);
        double bestF1Score = ComputeF1Score(communitySize, groundTruthCommunitySize, intersectionSize);
        for (std::vector<std::pair<unsigned int, std::vector <double> > >::iterator it = nodeLexRank.begin();
             it != nodeLexRank.end(); ++it) {
            unsigned int node = (*it).first;
            community.insert(node);
            communitySize++;
            if (groundTruthCommunity.find(node) != groundTruthCommunity.end()) {
                intersectionSize ++;
            }
            double f1Score = ComputeF1Score(communitySize, groundTruthCommunitySize, intersectionSize);
            if (f1Score >= bestF1Score) {
                bestCommunity = community;
                bestF1Score = f1Score;
            } else if (communitySize > 2 * groundTruthCommunitySize) {
                break;
            }
        }
    communities[i] = bestCommunity;
    f1Scores[i] = bestF1Score;
    }
    std::cout << std::endl;
    return 0;
}

int LexRankThreshold(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
                      std::vector <std::set<unsigned int> >& groundTruthCommunities,
                      std::vector <std::set<unsigned int> >& seeds,
                      std::vector <std::set<unsigned int> >& communities,
                      std::vector <double>& f1Scores,
                      std::vector <double>& lexRankThreshold) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        std::vector<std::pair<unsigned int, std::vector <double> > > nodeLexRank(lexRankResult[i]);
        std::set<unsigned int> community(seeds[i]);
        for (std::vector<std::pair<unsigned int, std::vector <double> > >::iterator it = nodeLexRank.begin();
             it != nodeLexRank.end(); ++it) {
            unsigned int node = (*it).first;
            std::vector <double> lexRank = (*it).second;
            if (lexRank > lexRankThreshold) {
                community.insert(node);
            } else {
                break;
            }
        }
    communities[i] = community;
    f1Scores[i] = ComputeF1Score(community, groundTruthCommunity);
    }
    return 0;
}

int LexRankThresholdFindBest(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& bestCommunities,
        std::vector <double>& bestF1Scores,
        unsigned int nbSteps) {
    std::vector <double> lexRankThreshold(nbSteps);
    double bestF1Score = 0.0;
    std::vector <double> bestThreshold(nbSteps);
    std::vector <std::set<unsigned int> > communities (bestCommunities.size());
    std::vector <double> f1Scores (bestF1Scores.size());
    for (int i = 0; i < 100; i++) {
        DisplayProgress(((double) i) / (double) 100, 100);
        lexRankThreshold[0] = 0.001 * ((double) i);
        LexRankThreshold(lexRankResult, groundTruthCommunities, seeds, communities, f1Scores, lexRankThreshold);
        double averageF1Score = GetAverage(f1Scores);
        if (averageF1Score > bestF1Score) {
            bestThreshold = lexRankThreshold;
            bestF1Score = averageF1Score;
            std::copy(communities.begin(), communities.end(), bestCommunities.begin());
            std::copy(f1Scores.begin(), f1Scores.end(), bestF1Scores.begin());
        }
    }
    std::cout << std::endl;
    printf ("Best threshold: %f\n", bestThreshold[0]);
    return 0;
}

int LexRankMinConductance(std::vector<std::set <unsigned int> >& nodeNeighbors,
        std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& communities,
        std::vector <double>& f1Scores) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        unsigned int groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector<std::pair<unsigned int, std::vector <double> > > nodeLexRank(lexRankResult[i]);
        std::set<unsigned int> community(seeds[i]);
        unsigned int communitySize = community.size();
        std::set<unsigned int> bestCommunity(community);
        unsigned int internalEdges = 0;
        unsigned int degreeSum = 0;
        for (std::set<unsigned int>::iterator it = community.begin(); it != community.end(); ++it) {
            unsigned int node = *it;
            std::set <unsigned int> neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
        }
        double bestConductance = ComputeConductance(degreeSum, internalEdges);


        for (std::vector<std::pair<unsigned int, std::vector <double> > >::iterator it = nodeLexRank.begin();
             it != nodeLexRank.end(); ++it) {
            unsigned int node = (*it).first;
            community.insert(node);
            communitySize++;
            std::set <unsigned int> neighborhood = nodeNeighbors[node];
            internalEdges += ComputeIntersectionSize(neighborhood, community);
            degreeSum += neighborhood.size();
            double conductance = ComputeConductance(degreeSum, internalEdges);
            if (conductance <= bestConductance) {
                bestCommunity = community;
                bestConductance = conductance;
            } else if (communitySize > 2 * groundTruthCommunitySize) {
                break;
            }
        }
        communities[i] = bestCommunity;
        f1Scores[i] = ComputeF1Score(bestCommunity, groundTruthCommunity);
    }
    std::cout << std::endl;
    return 0;
}

