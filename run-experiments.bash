#!/bin/bash

# config
EXEC="./bin/experiment"
TESTS="./bin/tests"
RANK_HEADER="problem,bitvector_size,num_rank_calls,num_iter,overhead,avg_duration"
SELECT_HEADER="problem,bitvector_size,num_select_calls,num_iter,overhead,avg_duration"
SPARSEARRAY_HEADER="problem,array_size,sparsity,num_func_calls,dense_overhead,sparse_overhead,avg_append_duration,\
                    avg_getatindex_duration,avg_getatrank_duration"

BOUNDS_CHECKING_FLAG="NO_BOUNDS_CHECKING=1"

# build
make clean
make ${BOUNDS_CHECKING_FLAG}
if [ $? -ne 0 ]; then
    echo "Build error. Stopping."
    exit 1
fi

# tests
${TESTS}
if [ $? -ne 0 ]; then
    echo "Failed tests. Stopping."
    exit 1
fi

# ===================
# === EXPERIMENTS ===
# ===================

# -- Rank --
echo "Running Rank experiment..."
echo ${RANK_HEADER} > rank-results.csv
for N in 100 1000 10000 100000 1000000; do
    ${EXEC} rank ${N} 100000 >> rank-results.csv
done

# -- Select --
echo "Running Select experiment..."
echo ${SELECT_HEADER} > select-results.csv
for N in 100 1000 10000 100000 1000000; do
    ${EXEC} select ${N} 100000 >> select-results.csv
done

# -- SparseArray --
echo "Running SparseArray experiment..."
echo ${SPARSEARRAY_HEADER} > sparsearray-results.csv
for N in 100 1000 10000 100000; do
    for f in 0.01 0.1 0.2 0.5 0.8; do
        ${EXEC} sparsearray ${N} ${f} 100000 >> sparsearray-results.csv
    done
done


# ======================
# === GENERATE PLOTS ===
# ======================
echo "Generating plots..."
python3 generate-plots.py rank-results.csv select-results.csv sparsearray-results.csv