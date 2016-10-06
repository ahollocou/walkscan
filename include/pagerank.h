#ifndef DYCOLA_PAGERANK_H
#define DYCOLA_PAGERANK_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>

bool nodePageRankCompare(const std::pair<unsigned int, double>& node1, const std::pair<unsigned int, double>& node2);
int PageRank(std::vector<std::set<unsigned int> >& nodeNeighbors,
             std::vector <std::set <unsigned int> >& seeds,
             unsigned int nbSteps, double alpha,
             std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
             unsigned int maxNodeId);
int PageRankMaxF1(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores,
                  std::vector <double>& pageRankThresholds);
int PageRankThreshold(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                      std::vector <std::set<unsigned int> >& groundTruthCommunities,
                      std::vector <std::set<unsigned int> >& seeds,
                      std::vector <std::set<unsigned int> >& communities,
                      std::vector <double>& f1Scores,
                      double pageRankThreshold);
int PageRankThresholdFindBest(std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                              std::vector <std::set<unsigned int> >& groundTruthCommunities,
                              std::vector <std::set<unsigned int> >& seeds,
                              std::vector <std::set<unsigned int> >& communities,
                              std::vector <double>& f1Scores);
int PageRankMinConductance(std::vector<std::set <unsigned int> >& nodeNeighbors,
                           std::vector <std::vector <std::pair<unsigned int, double> > >& pageRankResult,
                           std::vector <std::set<unsigned int> >& groundTruthCommunities,
                           std::vector <std::set<unsigned int> >& seeds,
                           std::vector <std::set<unsigned int> >& communities,
                           std::vector <double>& f1Scores);

#endif
