#include "../include/pagerank.h"
#include "../include/utils.h"

int PageRank(std::vector<std::set<unsigned int> >& nodeNeighbors,
             std::vector <std::set <unsigned int> >& seeds,
             unsigned int nbSteps, double alpha,
             std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
             unsigned int maxNodeId) {
    unsigned int counter = 0;
    unsigned int nbCommunities = seeds.size();
    for (std::vector <std::set <unsigned int> >::iterator it1 = seeds.begin();
         it1 != seeds.end(); ++it1) {
        DisplayProgress(((double) counter) / (double) nbCommunities, 100);
        std::set <unsigned int> seedSet(*it1);
        unsigned int seedSetSize = seedSet.size();
        std::set <unsigned int> walkSupport;
        std::vector <double> pageRank (maxNodeId + 1);
        std::vector <bool> isSeed (maxNodeId, false);
        // Initialization of the walk from the seed nodes
        for (std::set <unsigned int>::iterator it2 = seedSet.begin();
             it2 != seedSet.end(); ++it2) {
                pageRank[*it2] = 1.0 / ((double) seedSetSize);
                walkSupport.insert(*it2);
                isSeed[*it2] = true;
        }
        // For each step
        for (unsigned int t = 0; t < nbSteps; t++) {
            std::set <unsigned int> nextWalkSupport(walkSupport);
            std::vector <double> nextPageRank(pageRank.size());
            // For each node with a pagerank > 0 at the previous step
            for (std::set <unsigned int>::iterator it2 = walkSupport.begin();
                 it2 != walkSupport.end(); ++it2) {
                unsigned int node1 = *it2;
                // The walker stays in place with probability 1 - alpha
                nextPageRank[node1] += (1.0 - alpha) * pageRank[node1];
                std::set <unsigned int> neighbors = nodeNeighbors[node1];
                double degree = neighbors.size();
                for (std::set <unsigned int>::iterator it3 = neighbors.begin();
                     it3 != neighbors.end(); ++it3) {
                    // The walker goes to one of its neighbor with probability alpha * 1 / degree
                    unsigned int node2 = *it3;
                    nextPageRank[node2] += alpha * pageRank[node1] / degree;
                    nextWalkSupport.insert(node2);
                }
            }
            walkSupport = nextWalkSupport;
            pageRank = nextPageRank;
        }
        // Building output
        std::vector<std::pair<unsigned int, double> > nodePageRank;
        for (std::set <unsigned int>::iterator it2 = walkSupport.begin();
             it2 != walkSupport.end(); ++it2) {
            if (!isSeed[*it2]) {
                nodePageRank.push_back(std::make_pair(*it2, pageRank[*it2]));
            }
        }
        std::sort(nodePageRank.begin(), nodePageRank.end(), nodePageRankCompare);
        pageRankResult.push_back(nodePageRank);
        counter++;
    }
    std::cout << std::endl;
    return 0;
}

bool nodePageRankCompare(const std::pair<unsigned int, double>& node1, const std::pair<unsigned int, double>& node2) {
    return node1.second > node2.second;
}

int PageRankMaxF1(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores,
                  std::vector <double>& pageRankThresholds) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        unsigned int groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector<std::pair<unsigned int, double> > nodePageRank(pageRankResult[i]);
        std::set<unsigned int> community(seeds[i]);
        unsigned int communitySize = community.size();
        std::set<unsigned int> bestCommunity(community);
        unsigned int intersectionSize = ComputeIntersectionSize(community, groundTruthCommunity);
        double bestF1Score = ComputeF1Score(communitySize, groundTruthCommunitySize, intersectionSize);
        double lastPageRank = 1.0;
        for (std::vector<std::pair<unsigned int, double> >::iterator it = nodePageRank.begin();
             it != nodePageRank.end(); ++it) {
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
                lastPageRank = (*it).second;
            } else if (communitySize > 2 * groundTruthCommunitySize) {
                break;
            }
        }
    communities[i] = bestCommunity;
    f1Scores[i] = bestF1Score;
    pageRankThresholds[i] = lastPageRank;
    }
    std::cout << std::endl;
    return 0;
}

int PageRankThreshold(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                      std::vector <std::set<unsigned int> >& groundTruthCommunities,
                      std::vector <std::set<unsigned int> >& seeds,
                      std::vector <std::set<unsigned int> >& communities,
                      std::vector <double>& f1Scores,
                      double pageRankThreshold) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        std::vector<std::pair<unsigned int, double> > nodePageRank(pageRankResult[i]);
        std::set<unsigned int> community(seeds[i]);
        for (std::vector<std::pair<unsigned int, double> >::iterator it = nodePageRank.begin();
             it != nodePageRank.end(); ++it) {
            unsigned int node = (*it).first;
            double pageRank = (*it).second;
            if (pageRank > pageRankThreshold) {
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

int PageRankThresholdFindBest(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& bestCommunities,
        std::vector <double>& bestF1Scores) {
    double pageRankThreshold;
    double bestF1Score = 0.0;
    double bestThreshold = 0.0;
    std::vector <std::set<unsigned int> > communities (bestCommunities.size());
    std::vector <double> f1Scores (bestF1Scores.size());
    for (int i = 0; i < 100; i++) {
        DisplayProgress(((double) i) / (double) 100, 100);
        pageRankThreshold = 0.001 * ((double) i);
        PageRankThreshold(pageRankResult, groundTruthCommunities, seeds, communities, f1Scores, pageRankThreshold);
        double averageF1Score = GetAverage(f1Scores);
        if (averageF1Score > bestF1Score) {
            bestThreshold = pageRankThreshold;
            bestF1Score = averageF1Score;
            std::copy(communities.begin(), communities.end(), bestCommunities.begin());
            std::copy(f1Scores.begin(), f1Scores.end(), bestF1Scores.begin());
        }
    }
    std::cout << std::endl;
    printf ("Best threshold: %f\n", bestThreshold);
    return 0;
}

int PageRankMinConductance(std::vector<std::set <unsigned int> >& nodeNeighbors,
        std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& communities,
        std::vector <double>& f1Scores) {
    unsigned int nbCommunities = groundTruthCommunities.size();
    for (unsigned int i = 0; i < nbCommunities; i++) {
        DisplayProgress(((double) i) / (double) nbCommunities, 100);
        std::set<unsigned int> groundTruthCommunity(groundTruthCommunities[i]);
        unsigned int groundTruthCommunitySize = groundTruthCommunity.size();
        std::vector<std::pair<unsigned int, double> > nodePageRank(pageRankResult[i]);
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


        for (std::vector<std::pair<unsigned int, double> >::iterator it = nodePageRank.begin();
             it != nodePageRank.end(); ++it) {
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

