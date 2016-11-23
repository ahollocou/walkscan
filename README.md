walkscan
---

This repository provides a reference implementation of the local community detection algorithms
described in the following research paper submitted to an upcomming conference:

**Improving PageRank for Local Community Detection**, Alexandre Hollocou, Thomas Bonald, Marc Lelarge

ArXiv link: https://arxiv.org/abs/1610.08722

# C++

A C++ implementation of WalkSCAN, PageRank and LexRank is available in the **c_code** directory.

## Compilation

Execute the following commands in the **c_code** directory:

    mkdir build
    cd build
    cmake ..
    make

Note: For MacOS X, install GCC with Homebrew and use:

    cmake -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/6.2.0/bin/g++-6 ..

## Usage for local community detection

To detect communities from given seed sets, use the **walkscan** command:

    Usage: walkscan <flags>
    Availaible flags:
        -i [graph file name] : Specifies the graph file (tab-separated list of edges).
        -o [output path/prefix] : Specifies the prefix for output files.
        -s [seed file name] : Specifies a file with the seed sets.
        -t [walk length] : Specifies the length of the random walks (default value: 2).
        -a [algorithm] : Specifies the algorithm (default value: 0).
            0: PageRank with conductance
            1: LexRank with conductance
            2: WalkScan (use flags --epsilon and --min-elems to change parameter values)

## Usage for benchmarks

To perform benchmarks with ground-truth information, use the **benchmarks** command:

    Usage: benchmarks <flags>
    Availaible flags:
        -i [graph file name] : Specifies the graph file (tab-separated list of edges).
        -c [community file name] : Specifies the file with the ground-truth communities (tab-separated list of nodes).
        -b [benchmark type] :
            0 -> Classic benchmark: we pick a seed set in each ground-truth community.
            1 -> Random seed set: we pick k random seed nodes in the graph (and we make x simulations).
            2 -> Locally random seed set: we pick one seed set in the neighborhood of each ground-truth community.

#### Classic benchmark

    Usage: benchmarks <flags> - CLASSIC BENCHMARK
    Benchmark flags:
        -o [output path/prefix] : Specifies the prefix for output files.
        -s [seed file name] : Specifies a file with the seed sets (if not specified, seed nodes are chosen at random in each ground-truth community).
        -t [walk length] : Specifies the length of the random walks (default value: 2).
        -a [algorithm] : Specifies the algorithm (default value: 0).
            0: PageRank
            1: LexRank
            2: WalkScan (use flags --epsilon and --min-elems to change parameter values)
        -f [objective function] : Specifies the objective function for the sweep algorithm of PageRank or LexRank (default value: 0).
            0: Max-F1
            1: Rank Threshold
            2: Conductance

#### Random seed set benchmark

    Usage: benchmarks <flags> - RANDOM SEED SET
    Benchmark flags:
        -o [output path/prefix] : Specifies the prefix for output files.
        -s [seed file name] : Specifies a file with the seed sets (if not specified, k random seeds will be picked for each simulation).
        -k [number of seeds] : Specifies the number of seeds that will be picked at random at each simulation.
        -x [number of simulations]: Specifies the number of simulations.
        -t [walk length] : Specifies the length of the random walks (default value: 2).
        -a [algorithm] : Specifies the algorithm(default value: 0).
            0: PageRank (with conductance as objective function)
            2: WalkScan (use flags --epsilon and --min-elems to change parameter values)

#### Locally random seed set benchmark

    Usage: benchmarks <flags> - LOCALLY RANDOM SEED SET
    Benchmark flags:
        -o [output path/prefix] : Specifies the prefix for output files.
        -s [seed file name] : Specifies a file with the seed sets (if not specified, seeds are chosen at random in the neighborhood of ground-truth communities).
        -d [distance to ground-truth] : Specifies the maximum distance from the ground truth communities where seed nodes can be picked.
        -t [walk length] : Specifies the length of the random walks (default value: 2).
        -a [algorithm] : Specifies the algorithm (default value: 0).
            0: PageRank
            1: LexRank
            2: WalkScan (use flags --epsilon and --min-elems to change parameter values)
        -f [objective function] : Specifies the objective function for the sweep algorithm of PageRank or LexRank (default value: 0).
            0: Max-F1
            1: Rank Threshold
            2: Conductance

# Python

An implementation of WalkSCAN in Python is available in the **python_code** directory.

## Dependencies
WalkSCAN requires the following packages:
[NetworkX](https://pypi.python.org/pypi/networkx/)
, [NumPy](https://pypi.python.org/pypi/numpy/)
and [Scikit Learn](https://pypi.python.org/pypi/scikit-learn/).



## Example

```{python}
from walkscan import WalkSCAN
import networkx as nx

# Create a random graph with two partitions
G = nx.random_partition_graph([10, 15], 0.9, 0.01)

# Create a WalkSCAN instance
ws = WalkSCAN(nb_steps=3, eps=0.05, min_samples=2)

# Initialization vector for the random walk
init_vector = {0: 0.5, 1: 0.5}

# Compute the communities
ws.detect_communities(G, init_vector)

# Print the best community
print ws.communities_[0]
```

# Usage

The class constructor of `WalkSCAN` takes three parameters:

- the length of the random walk: `nb_steps`
- the two parameters for DBSCAN `eps` and `min_samples`

In order to run the community detection algorithm, use the `detect_communities` method with parameters:

- `graph`: NetworkX graph (can be weighted)
- `init_vector`: dictionary `node_id -> initial_probability` to initialize the random walk

The results of the algorithm are stored in the attributes:

- `communities_`: list of communities, ordered by decreasing closeness to the seed set
- `embedded_nodes_`: list of the embedded nodes
- `embedded_values_`: dictionary `node_id -> embedding_value` containing the result of the embedding
- `cores_`: dictionary containing the cores computed via DBSCAN
- `outliers_`: list of the outliers computed by DBSCAN

# Citing

If you find WalkSCAN interesting for your research, please consider citing our paper.

# Contact

Please send any questions you might have about the paper or the code to <alexandre.hollocou@inria.fr>.
