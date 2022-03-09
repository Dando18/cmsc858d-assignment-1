# Task 2
--------
This task implements the `SelectSupport` class.

`SelectSupport` is implemented with the same API as in the assignment listing.
It is the last class defined in `bitvector.h`.
This file also defines `BitVector`, `PackedVector`, and `RankSupport` classes.

The constructor takes a reference to an existing `RankSupport`.
To compute the index of a rank you can either call `SelectSupport::select1` or `SelectSupport::operator()`.
The load and save functions do not do anything.

To test the select support go into the parent directory, run `make`, and then 
`./bin/experiment select bitvectorSize numSelectCalls`.
This will create a bitvector of _bitvectorSize_ size and call select _numSelectCalls_ times.
The timings and overheads will be averaged over 50 runs and printed out to stdout in a CSV format.