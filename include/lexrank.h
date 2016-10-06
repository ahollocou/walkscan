#ifndef DYCOLA_LEXRANK_H
#define DYCOLA_LEXRANK_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>

int LexRank(std::vector<std::set<unsigned int> >& nodeNeighbors,
            std::vector <std::set <unsigned int> >& seeds,
            unsigned int nbSteps,
            std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
            unsigned int maxNodeId);
bool nodeLexRankCompare(const std::pair<unsigned int, std::vector <double> >& node1,
                        const std::pair<unsigned int, std::vector <double> >& node2);
int LexRankMaxF1(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores);
int LexRankThreshold(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
                      std::vector <std::set<unsigned int> >& groundTruthCommunities,
                      std::vector <std::set<unsigned int> >& seeds,
                      std::vector <std::set<unsigned int> >& communities,
                      std::vector <double>& f1Scores,
                      std::vector <double>& lexRankThreshold);
int LexRankThresholdFindBest(std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& bestCommunities,
        std::vector <double>& bestF1Scores,
        unsigned int nbSteps);
int LexRankMinConductance(std::vector<std::set <unsigned int> >& nodeNeighbors,
        std::vector <std::vector <std::pair<unsigned int, std::vector <double> > > >& lexRankResult,
        std::vector <std::set<unsigned int> >& groundTruthCommunities,
        std::vector <std::set<unsigned int> >& seeds,
        std::vector <std::set<unsigned int> >& communities,
        std::vector <double>& f1Scores);

#endif
