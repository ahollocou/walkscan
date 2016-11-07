# walkscan

This repository provides a reference implementation of the local community detection algorithms
described in the following research paper submitted to an upcomming conference:

**Improving PageRank for Local Community Detection**, Alexandre Hollocou, Thomas Bonald, Marc Lelarge

ArXiv link: https://arxiv.org/abs/1610.08722

## Compilation

Be sure to have downloaded the DBSCAN submodule:
```shell
git submodule update --init --recursive
```

```shell
cmake .
make
```

Note: For MacOS X, install GCC with Homebrew and use:

```shell
cmake -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/6.2.0/bin/g++-6 .
```

## Usage

    Graph filenameme not set
    Usage: walkscan <flags>
    Availaible flags:
        -i [graph file name] : Specifies the graph file (tab-separated list of edges).
        -c [community file name] : Specifies the file with the ground-truth communities (tab-separated list of nodes).
        -b [benchmark type] :
            0 -> Classic benchmark: we pick a seed set in each ground-truth community.
            1 -> Random seed set: we pick k random seed nodes in the graph (and we make x simulations).
            2 -> Locally random seed set: we pick one seed set in the neighborhood of each ground-truth community.

### Classic benchmark

    Usage: walkscan <flags> - CLASSIC BENCHMARK
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

### Random seed set benchmark

    Usage: walkscan <flags> - RANDOM SEED SET
    Benchmark flags:
        -o [output path/prefix] : Specifies the prefix for output files.
        -s [seed file name] : Specifies a file with the seed sets (if not specified, k random seeds will be picked for each simulation).
        -k [number of seeds] : Specifies the number of seeds that will be picked at random at each simulation.
        -x [number of simulations]: Specifies the number of simulations.
        -t [walk length] : Specifies the length of the random walks (default value: 2).
        -a [algorithm] : Specifies the algorithm(default value: 0).
            0: PageRank (with conductance as objective function)
            2: WalkScan (use flags --epsilon and --min-elems to change parameter values)

### Locally random seed set benchmark

    Usage: walkscan <flags> - LOCALLY RANDOM SEED SET
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

## Citing

If you find WalkSCAN interesting for your research, please consider citing our paper.

## Contact

Please send any questions you might have about the paper or the code to <alexandre.hollocou@inria.fr>.
