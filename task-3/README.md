# Task 3
--------
This task implements the `SparseArray` class.

`SparseArray` is implemented with the same API as in the assignment listing.
It is defined in `SparseArray.h`.

`SparseArray::create(uint64_t)` will allocate memory for a sparse array of a given size.

`SparseArray::append` or `SparseArray::emplace` can be used to append elements into the sparse array.

To test the sparse array go into the parent directory, run `make`, and then 
`./bin/experiment sparsearray arraySize sparsity numFuncCalls`.
This will create a SparseArray of _arraySize_ size, fill it up to _sparsity_ fraction (i.e. 0.1=10%), and
call each function _numFuncCalls_ times.
The timings and overheads will be averaged over 50 runs and printed out to stdout in a CSV format.