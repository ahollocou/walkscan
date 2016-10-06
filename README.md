# walkscan

This repository provides a reference implementation of the local community detection algorithms
described in a research paper submitted to an upcomming data-mining conference.

## Compilation

Be sure to have downloaded the DBSCAN submodule:
```shell
git submodule update --init --recursive
```

```shell
cmake .
make
```

For MacOS X, install GCC with Homebrew and use:

```shell
cmake -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/6.2.0/bin/g++-6 .
```
