# An Assignment-based Local Search for Maximum Independent Union of Cliques Problem
The software and data in this repository are a snapshot of the software and data that were used in the research reported on in the paper An Assignment-based Local Search for Maximum Independent Union of Cliques Problem by Yiping Liu, Xinyu Wang and Yi Zhou.

# Description
Given an undirected graph $G=(V,E)$, an independent union of cliques (IUC) is a subset of vertices $S\subseteq V$ such that each connected component of the subgraph induced by $S$ is a complete graph. The maximum IUC problem asks to find an IUC with the maximum number of vertices.  An example of The maximum IUC problem is shown in the following Figure. 

<p align="center">
  <img src="https://github.com/Leaflowave/ALS/blob/main/aaa.png?raw=true"  alt="Sublime's custom image" />
</p>

# Prerequisites
The codes are implemented under Ubuntu 22.04. Boost C++ 17 is also required for running the codes.

# Build and Run
In Linux/Ubuntu operating system, execute the following command to build the program.
```
g++ ./LDTPS/src/main.cpp ./LDTPS/src/common_func_def.cpp ./LDTPS/src/local_search.cpp -o ./LDTPS/IUC -O3
```
