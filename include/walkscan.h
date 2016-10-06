#ifndef DYCOLA_WALKSCAN_H
#define DYCOLA_WALKSCAN_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <stdint.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <dbscan.h>

int WalkScan(std::vector<std::set<unsigned int> >& nodeNeighbors,
             std::vector <std::set<unsigned int> >& groundTruthCommunities,
             std::vector <std::set <unsigned int> >& seeds,
             unsigned int nbSteps,
             std::vector <std::vector <std::set<unsigned int> > > & walkScanResult,
             unsigned int maxNodeId,
             double epsilon,
             unsigned int minElems);
int WalkScanMaxF1(std::vector <std::vector <std::set<unsigned int> > > & walkScanResult,
                  std::vector <std::set<unsigned int> >& groundTruthCommunities,
                  std::vector <std::set<unsigned int> >& seeds,
                  std::vector <std::set<unsigned int> >& communities,
                  std::vector <double>& f1Scores);

#endif
