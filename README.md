# Assignment 1
https://rob-p.github.io/CMSC858D_S22/assignments/01_assignment_01

`make` will build the code. 
`make DEBUG=1` builds a debug version. 
`make NO_BOUNDS_CHECKING=1` turns off bounds checking in the build (i.e. no `throw ...;` calls in the methods).

`./bin/tests` will run a set of tests on BitVector, RankSupport, SelectSupport, and SparseArray.

`./bin/run` is used for the experiments to collect results.