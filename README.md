# Assignment 1
https://rob-p.github.io/CMSC858D_S22/assignments/01_assignment_01

## Building and Running

`make` will build the code. 
`make DEBUG=1` builds a debug version. 
`make NO_BOUNDS_CHECKING=1` turns off bounds checking in the build (i.e. no `throw ...;` calls in the methods).

`./bin/tests` will run a set of tests on BitVector, RankSupport, SelectSupport, and SparseArray.
The program will print and exit with non-zero code if a test fails.

`./bin/experiment` is used for the experiments to collect results. 
The program will write a csv line with results to stdout on completion.
It can be run with three options:

```sh
# Run and time a bunch of rank calls
./bin/experiment rank bitvectorSize numRankCalls

# Run and time a bunch of select calls
./bin/experiment select bitvectorSize numSelectCalls

# Run and time all methods of SparseArray with specified sparsity
./bin/experiment sparsearray bitvectorSize sparsity numFuncCalls
```

Running `bash run-experiments.bash` will build the code, run a set of experiments, and generate plots.
It can be used to reproduce the reported results.


## Project and Code Layout

`include/` contains most of the source code.
`bitvector.h` implements `BitVector`, `PackedVector`, `RankSupport`, and `SelectSupport`.
`sparsearray.h` implements `SparseArray<T>`.
`utilities.h` contains several bit manipulation and serialization utility functions.

`src/` holds the testing program `test.cc` and experiment driver `experiment.cc`.

`bin/` is where the executables get stored.

`data/` is created by `run-experiments.bash` and will contain the csv files with results.

`figs/` is created by `generate-plots.py` and stores the figures for the experiment results.

