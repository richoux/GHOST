# Changelog

All notable changes to this project will be documented in this file, since GHOST 2.0.0.

## [3.2.0] - 2025-06-04
- Changed `Options.percent_chance_escape_plateau` with `Options.percent_chance_force_trying_on_plateau`, as well as some printed traces.
- Add `Options.enable_optimization_guidance` to enable (default) or disable the usage of the objective function as a tie-breaker for satisfaction plateau.

## [3.1.0] - 2024-03-11
- Add Random Walk and Hill Climbing as search algorithms, for landscape analysis.
- Minor changes in the global solver algorithm, such as removing the probability to force tabu-marking a variable. 

## [3.0.2] - 2023-09-26
- Fix a mistake with maximization objectives in the complete solver, and a bug that happened with the compiler Clang.

## [3.0.1] - 2023-09-21
- Hotfix adding a method to call the complete solver with default options.

## [3.0.0] - 2023-09-20
- Add a complete solver within the framework, implementing the Arc Consistency 3 algorithm and aiming to find all solutions of a problem instance.
- Changed the interface `Solver::solve` -> `Solver::fast_search`, to be coherent with the new `Solver::complete_search`

## [2.8.2] - 2023-07-21
-  Fix a mistake in the final cost of an objective function to maximize, where the value v needed to be reset to the opposite value -v.

## [2.8.1] - 2023-06-22
-  Change Cmake and build.sh to compile the library for different Android architectures.

## [2.8.0] - 2023-05-31
- Now allow modeling pure optimization problems.

## [2.7.0] - 2023-05-30
- Slight memory management improvement, by pre-allocating memory during the search.

## [2.6.0] - 2023-01-24
- Add the AllEqual global constraint.

## [2.5.2] - 2022-09-21
- Add virtual destructors in base classes. This fixes a problem on OSX.

## [2.5.1] - 2022-07-11
- Fix minor forgettings (Doxygen file, README, ...).

## [2.5.0] - 2022-07-11
- Add all possible linear equation global constraints.

## [2.4.0] - 2022-07-08
- Error projection is now implemented following the Strategy pattern.

## [2.3.1] - 2022-06-29
- Fix an error in the AllDifferent global constraint.
- Add a constructor taking an argument std::vector<ghost::Variable> for each global constraint.

## [2.3.0] - 2022-06-23
- Add first global constraints: AllDifferent, FixValue and LinearEquation.
- Search unit inner data have been moved into an object.
- Variable and value selection heuristics are now implemented following the Strategy pattern.
- Code optimization (avoiding copies of randutils::mt19937_rng objects)
- Change the file tree

## [2.2.1] - 2021-12-13
- Fix mistakes within the search unit when one starts with a custom sector of variable values.
- Fix a mistake in Options documentation.

## [2.2.0] - 2021-11-10
- The default number of threads is now the number of physical CPU cores (half the number of virtual cores for CPUs with hyper-threading).
- Add in `Options` a parameter to set the percentage of chance to escape a plateau rather than exploring it.
- Rename `Options::percent_to_reset` to `Options::number_variables_to_reset`.

## [2.1.2] - 2021-11-02
- Enable the include and lib files installation on Windows with Visual Studio.

## [2.1.1] - 2021-10-18
- Fix a bug in the solver when some variables are assigned to any constraints.
- Fix a mistake in Objective in case some vectors are empty.

## [2.1.0] - 2021-10-04
This is a minor version change due to some interface modifications.

- Permutation problem declaration is now done in the `ModelBuilder` constructor.
- Remove `Objective::expert_postprocess_satisfaction` and change the name and the interface of `Objective::expert_postprocess` (previously `Objective::expert_postprocess_optimization`)

## [2.0.1] - 2021-09-30
- Fix a bad implementation of the copy-and-swap idiom in ghost::Options
- Fix some typos in the Solver screen outputs when the macro GHOST_BENCH is declared
- Remove unnecessary whitespaces

## [2.0.0] - 2021-09-29

Major refactoring of version 1.0.1
- deep interface modification
- performance improvements
- new features (parallel search, ...)
