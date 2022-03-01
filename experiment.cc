/* For performance testing.
author: Daniel Nichols
date: February 2022
*/
// stl includes
#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <string>

// local includes
#include "bitvector.h"
#include "sparsearray.h"

/* average results over this number of tests. */
constexpr uint32_t NUM_TEST_ITER = 50;

/* declarations */
void testRank(uint64_t bvSize, uint64_t numRankCalls);
void testSelect(uint64_t bvSize, uint64_t numSelectCalls);
void testSparseArray(uint64_t size, float sparsity, uint64_t funcCalls);

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << "<rank|select|sparsearray> <options...>\n";
        return 1;
    }

    std::string action(argv[1]);
    std::transform(std::begin(action), std::end(action), std::begin(action), ::tolower);
    if (action == "rank") {
        if (argc != 4) {
            std::cerr << "usage: " << argv[0] << "rank bitvectorSize numRankCalls\n";
            return 1;
        }

        const uint64_t bvSize = std::stoull(std::string(argv[2]));
        const uint64_t numRankCalls = std::stoull(std::string(argv[3]));

        testRank(bvSize, numRankCalls);
    } else if (action == "select") {
        if (argc != 4) {
            std::cerr << "usage: " << argv[0] << "select bitvectorSize numSelectCalls\n";
            return 1;
        }

        const uint64_t bvSize = std::stoull(std::string(argv[2]));
        const uint64_t numSelectCalls = std::stoull(std::string(argv[3]));

        testSelect(bvSize, numSelectCalls);
    } else if (action == "sparsearray" || action == "sparse-array") {
        if (argc != 5) {
            std::cerr << "usage: " << argv[0] << "select bitvectorSize sparsity numFuncCalls\n";
            return 1;
        }

        const uint64_t bvSize = std::stoull(std::string(argv[2]));
        const float sparsity = std::stof(std::string(argv[3]));
        const uint64_t numFuncCalls = std::stoull(std::string(argv[4]));

        if (sparsity <= 0.0 || sparsity > 1.0) {
            std::cerr << "sparsity must be in (0,1]." << "\n";
            return 1;
        }

        testSparseArray(bvSize, sparsity, numFuncCalls);
    } else {
        std::cerr << "usage: " << argv[0] << "<rank|select|sparsearray> <options...>\n";
        return 1;
    }
}


void testRank(uint64_t bvSize, uint64_t numRankCalls) {
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<uint64_t> dist{0, bvSize-1};

    uint64_t overhead = 0;
    double avgDuration = 0.0;

    for (uint32_t i = 0; i < NUM_TEST_ITER; i += 1) {

        const bitvector::BitVector bv = bitvector::getRandomBitVector(bvSize);
        bitvector::RankSupport rank(bv);

        /* record overhead */
        if (i == 0) {
            overhead = rank.overhead();
        }

        /* generate random indices ahead of time */
        std::vector<uint64_t> indices(numRankCalls);
        std::generate(std::begin(indices), std::end(indices), [&rng, &dist](){ return dist(rng); });

        const auto begin = std::chrono::high_resolution_clock::now();
        for (uint64_t j = 0; j < numRankCalls; j += 1) {
            /*  I've experimentally confirmed on clang and gcc with -S flag that this does not get optimized away.
                Probably since RankSupport accesses bitvector's data, which may be aliased...? */
            rank(indices[j]);
        } 
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration<double>(end-begin).count();
        avgDuration += duration;
    }

    avgDuration /= static_cast<double>(NUM_TEST_ITER);

    std::cout << "rank," << bvSize << "," << numRankCalls << "," << NUM_TEST_ITER << "," << overhead << "," 
            << avgDuration << "\n";
}

void testSelect(uint64_t bvSize, uint64_t numSelectCalls) {
    std::random_device device;
    std::mt19937 rng(device());

    uint64_t overhead = 0;
    double avgDuration = 0.0;

    for (uint32_t i = 0; i < NUM_TEST_ITER; i += 1) {

        auto bitString = bitvector::getRandomBinaryString(bvSize);
        std::shuffle(std::begin(bitString), std::end(bitString), rng);

        const bitvector::BitVector bv(bitString);
        const bitvector::RankSupport rank(bv);
        bitvector::SelectSupport select(rank);
        std::uniform_int_distribution<uint64_t> dist{1, rank.totalOnes()};

        /* record overhead */
        if (i == 0) {
            overhead = select.overhead();
        }

        /* generate random indices ahead of time */
        std::vector<uint64_t> indices(numSelectCalls);
        std::generate(std::begin(indices), std::end(indices), [&rng, &dist](){ return dist(rng); });

        const auto begin = std::chrono::high_resolution_clock::now();
        for (uint64_t k = 0; k < numSelectCalls; k += 1) {
            /*  I've experimentally confirmed on clang and gcc with -S flag that this does not get optimized away.
                Probably since SelectSupport accesses bitvector's data, which may be aliased...? */
            select(indices[k]);
        } 
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration<double>(end-begin).count();
        avgDuration += duration;
    }

    avgDuration /= static_cast<double>(NUM_TEST_ITER);

    std::cout << "select," << bvSize << "," << numSelectCalls << "," << NUM_TEST_ITER << "," << overhead << "," 
            << avgDuration << "\n";
}

void testSparseArray(uint64_t size, float sparsity, uint64_t funcCalls) {

    const uint64_t numToInsert = static_cast<uint64_t>(size*sparsity);
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<uint64_t> dist, indexDist{0, size-1}, rankDist{0, numToInsert-1};
    auto getRandIndex = [&rng, &indexDist](){ return indexDist(rng); };
    auto getRandRank = [&rng, &rankDist](){ return rankDist(rng); };
    auto getRandKeyValuePair = [&rng, &dist, &indexDist](){ return std::make_pair(indexDist(rng), dist(rng)); };

    double avgAppendDuration = 0.0, avgGetAtIndexDuration = 0.0, avgGetAtRankDuration = 0.0;
    uint64_t sparseOverhead;
    const uint64_t denseOverhead = 8*sizeof(uint64_t)*numToInsert;

    for (uint32_t i = 0; i < NUM_TEST_ITER; i += 1) {

        sparse::SparseArray<uint64_t> array;
        array.create(size);
        if (i == 0) {
            sparseOverhead = array.overhead();
        }

        /* append */
        std::map<uint64_t, uint64_t> randVals;
        std::generate_n(std::inserter(randVals, std::begin(randVals)), numToInsert, getRandKeyValuePair);

        auto begin = std::chrono::high_resolution_clock::now();
        for (auto const& [key, value] : randVals) {
            array.append(value, key);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end-begin).count();
        avgAppendDuration += duration;

        /* get at index */
        uint64_t tmpVal;
        std::vector<uint64_t> indicesToQuery(funcCalls);
        std::generate(std::begin(indicesToQuery), std::end(indicesToQuery), getRandIndex);

        begin = std::chrono::high_resolution_clock::now();
        for (auto const& index : indicesToQuery) {
            array.getAtIndex(index, tmpVal);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(end-begin).count();
        avgGetAtIndexDuration += duration;

        /* get at rank */
        std::vector<uint64_t> ranksToQuery(funcCalls);
        std::generate(std::begin(ranksToQuery), std::end(ranksToQuery), getRandRank);

        begin = std::chrono::high_resolution_clock::now();
        for (auto const& rank : ranksToQuery) {
            array.getAtRank(rank, tmpVal);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(end-begin).count();
        avgGetAtRankDuration += duration;
    }

    avgAppendDuration /= static_cast<double>(NUM_TEST_ITER);
    avgGetAtIndexDuration /= static_cast<double>(NUM_TEST_ITER);
    avgGetAtRankDuration /= static_cast<double>(NUM_TEST_ITER);

    std::cout << "sparsearray," << size << "," << sparsity << "," << funcCalls << "," << denseOverhead << "," << 
            sparseOverhead << "," << avgAppendDuration << "," << avgGetAtIndexDuration << "," << 
            avgGetAtRankDuration << "\n";
}
