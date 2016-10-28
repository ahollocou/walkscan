#ifndef TYPES_H
#define TYPES_H

#include <set>

typedef std::set< uint32_t > NodeSet;
typedef std::pair< uint32_t, uint32_t > Edge;
typedef std::pair< uint32_t, double > NodePageRank;
typedef std::pair< uint32_t, std::vector< double > > NodeLexRank;
typedef std::pair< NodeSet, std::vector< double > > NodeSetLexRank;

#endif
