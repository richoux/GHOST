---
title: 'GHOST: A C++ framework to model and solve combinatorial optimization problems'
tags: [C++, combinatorial optimization, constraint programming, metaheuristics, local search]
authors:
  - name: Florian Richoux
    orcid: 0000-0003-4693-379X
    corresponding: true
    affiliation: 1
affiliations:
  - name: Artificial Intelligence Research Center, National Institute of Advanced Industrial Science and Technology (AIST), Japan
    index: 1
    ror: 04t4c5k48
date: 14 September 2026
bibliography: paper.bib
---

# Summary

`GHOST` (General meta-Heuristic Optimization Solving Toolkit) is an open-source C++17 framework for modeling and solving discrete combinatorial problems. It supports Constraint Satisfaction Problems (CSPs), Constrained Optimization Problems (COPs), and related formulations in which error functions quantify constraint violations. Users declare variables, constraints, and, optionally, an objective function, while `GHOST` supplies a constraint-based local search solver.

`GHOST` was initiated in 2014 for a *StarCraft: Brood War* bot, where all decisions for a game frame had to be computed in less than 100 ms. It has since supported research on decision making with short time budgets, terrain analysis, automatic error-function learning, Quadratic Unconstrained Binary Optimization (QUBO) reformulation, and combinations of combinatorial optimization with Monte Carlo Tree Search [@richoux2016ghost; @antuori2019uncertainty; @richoux2022taunt; @richoux2023amai; @richoux2023qubo; @richoux2025boop]. Its benchmark repository includes classic combinatorial (optimization) problems, such as Sudoku, N-Queens, the knapsack problem, magic squares, the quadratic assignment problem, the traveling salesman problem, the traveling tournament problem, and vertex cover [@ghost_benchmarks].

# Statement of need

Combinatorial optimization software involves trade-offs between modeling convenience, control over search, solution quality, and response time. Complete solvers, i.e., solvers exploring the whole search space, are valuable when optimality or infeasibility proofs are required, but such guarantees may be incompatible with interactive applications. Conversely, implementing a specialized metaheuristic provides control but requires substantial effort and often entangles the problem model with the search procedure.

`GHOST` addresses the intermediate need for a reusable native C++ framework that accepts constraint-based models and provides a ready-to-use metaheuristic solver. Its intended users are researchers and C++ developers who need feasible and improving solutions under limited computation budgets, wish to test alternative models without repeatedly implementing a solver, or need to embed combinatorial decision making in a C++ system. Relevant domains include decision-making, games, scheduling, logistics, routing, assignment, and resource allocation.

Constraints in `GHOST` have non-negative error functions, for which zero denotes satisfaction. The solver therefore obtains information about how far a candidate is from feasibility rather than only a Boolean signal. Because Adaptive Search is a metaheuristics, `GHOST` is an anytime solver: it returns its best solution when the allocated time expires, but cannot prove optimality or infeasibility.

# State of the field

Constraint programming systems such as Choco [@prudhomme2022choco] and OR-Tools CP-SAT [@ortools] provide expressive models and systematic solving techniques. They are natural choices when strong propagation, bounds, or proof-oriented solving are principal requirements. Comet introduced language-level support for constraint-based local search [@vanhentenryck2005comet], while OscaR.cbls provides a Scala framework for composing models, invariants, neighborhoods, and search procedures [@oscarcbls].

The Constraint programming community mainly proposes different languages to declare problem models, such as [XCSP3](http://xcsp.org/) and [MiniZinc](https://www.minizinc.org/resources.html), and compiled a large [catalogue](https://sofdem.github.io/gccat/) of more than 400 global constraints that users are encouraged to use in order to achieve the best solver performance. Originally, these global constraints aimed to isolate the most frequently used constraints and optimize their internal mechanisms, so that users would not have to reinvent the wheel and would have a comprehensive set of various constraints at their disposal to express their problems. However, we believe that the adoption of specialized languages—rather than libraries written in general-purpose programming languages—combined with an overly broad set of global constraints requiring genuine expertise to master, has hindered rather than helped the democratization of constraint programming in both industry and scientific research, paving the way for other approaches.

This is why `GHOST` occupies a different design point from other tools in the field. It is a lightweight C++ framework intended for direct integration into native applications and for problems with short, explicit time budgets. Users provide a constraint-level model while the framework supplies generic search control. This reduces the need to implement neighborhood exploration and metaheuristic logic for every application, while retaining extensibility through custom constraints and objective functions. Its priorities are ease of modeling and use, then execution speed. Users can define their own, custom constraints, but also use predefined constraints among a restricted set of the most common ones, such as AllDifferent, Linear Equation, and their variants. `GHOST` therefore complements rather than replaces exact solvers and more configurable local-search toolkits.

# Software design

The main `GHOST` solver implements Adaptive Search, a constraint-based local search algorithm [@codognet2001adaptive]. From a current assignment, its neighborhood contains every candidate reachable by changing one variable's value. It evaluates the constraint error functions and project on variables the error of each constraint a variable interact with. The search for solutions is guided by changing the assignment of variables with the largest error. After finding a feasible solution, it seeks better solutions according to the objective. When search appears trapped in a local optimum, it performs either a reset, randomly reassigning some variables, or a restart, randomly reassigning all variables.

The architecture separates problem models from search, rooting its phylosophy into the "holy grail" of Constraint Programming: "Users declare the problem, the computer solves it." A model defines variables and domains, constraints, and an optional objective. Satisfaction and optimization share these abstractions, and permutation and non-permutation representations are supported.

In addition of the constraint-based local search solver, a simple non-backtracking complete solver was later added to enumerate all solutions of an instance. This was motivated by the AI agent playing the board game *boop.* on an Android app, where enumeration was needed within Monte Carlo Tree Search [@richoux2025boop]. The framework's evolution is documented in its change log [@ghost_software].

`GHOST` is built with CMake and distributed under GNU GPL v3. The repository includes installation instructions for GNU/Linux, macOS, and Windows, tutorials, an introduction to constraint programming, Doxygen API documentation, automated tests, continuous integration, releases, a PDF cheat sheet, and a 30-minute introductory video [@ghost_software; @ghost_video]. A separate repository provides executable benchmarks [@ghost_benchmarks].

# Research impact statement

`GHOST` has been developed openly since 2014. The original work modeled target selection, wall-in construction, build-order planning, and resource allocation for real-time strategy games [@richoux2016ghost]. Further work studied robustness and flexibility on pathfinding and alternative resource-allocation models [@fradin2015robustness].

The software has subsequently enabled research on decision making under uncertainty and the `microPhantom` bot for *microRTS* [@antuori2019uncertainty; @richoux2020microphantom], constraint-based terrain analysis in *StarCraft* 1 and 2 [@richoux2022taunt], learning interpretable error functions [@richoux2021gecco; @richoux2023amai], learning QUBO constraint representations [@richoux2023qubo], and injecting combinatorial optimization into Monte Carlo Tree Search for *boop.* [@richoux2025boop].

`GHOST` is integrated into `pobo`, an Android implementation of *boop.*; `Taunt`, a C++ terrain-analysis library for *StarCraft* 1 and 2; and `microPhantom`, a bot for *microRTS*. It has also supported research systems for learning constraint error functions and QUBO reformulations. These applications influenced its evolution, including complete solution enumeration.

# AI usage disclosure

Microsoft 365 Copilot was used to prepare and revise this manuscript from the repositories, JOSS guidance, bibliographic sources, and information supplied by the author. The author reviewed the text and remains responsible for it. No generative AI was used to produce the `GHOST` source code or documentation.

# Acknowledgements

`GHOST` was developed while the author held positions at the French National Centre for Scientific Research (CNRS), Nantes University, and the National Institute of Advanced Industrial Science and Technology (AIST). These organizations did not provide dedicated funding specifically for `GHOST`.

# References
