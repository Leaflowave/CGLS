# CGLS: A Column Generation Heuristic for Maximum Independent Union of Cliques Problem
The software and data in this repository are a snapshot of the software and data that were used in the research reported on in the paper "CGLS: A Column Generation Heuristic for Maximum Independent Union of Cliques Problem".

# Description
Given an undirected graph $G=(V,E)$, an independent union of cliques (IUC) is a subset of vertices $S\subseteq V$ such that each connected component of the subgraph induced by $S$ is a complete graph. The maximum IUC problem asks to find an IUC with the maximum number of vertices.  An example of The maximum IUC problem is shown in the following Figure. 

<p align="center">
  <img src="https://github.com/Leaflowave/ALS/blob/main/IUC.png?raw=true"  alt="Sublime's custom image" />
</p>

# Prerequisites
The codes are implemented under Ubuntu 22.04. Boost C++ 17 is also required for running the codes.

# Build and Run
In Linux/Ubuntu operating system, execute the following command to build the program.
```
./compile.sh
```
To run the executable file, call the following command.
```
./build/IUC ./Instances/[dataset name].clq -1 <seed>
```
Be sure to clean all the dependencies and executable files before building a different version of the code.
