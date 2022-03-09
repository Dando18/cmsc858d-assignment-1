# Task 1
--------
This task implements the `RankSupport` class.

`RankSupport` is implemented with the same API as in the assignment listing.
It is the last class defined in `bitvector.h`.
This file also defines `BitVector` and `PackedVector` classes.

The constructor takes a reference to an existing `BitVector`.
On construction it creates the tables.
To compute the rank of an index you can either call `RankSupport::rank1` or `RankSupport::operator()`.
To store/load the rank tables use `RankSupport::save` and `RankSupport::load`.

`RankSupport::buildTables(uint64_t)` will rebuild the tables starting from the provided index.

To test the rank support go into the parent directory, run `make`, and then 
`./bin/experiment rank bitvectorSize numRankCalls`.
This will create a bitvector of _bitvectorSize_ size and call rank _numRankCalls_ times.
The timings and overheads will be averaged over 50 runs and printed out to stdout in a CSV format.