# Changelog

All notable changes to this project will be documented in this file, since GHOST 2.0.0.

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
