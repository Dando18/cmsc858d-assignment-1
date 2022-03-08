/*  Tests for bitvector, rank, select, sparsearray implementations.
    These are not exhaustive unit tests, but more of sanity checks.
    author: Daniel Nichols
    date: February 2022
*/
// stl includes
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

// local includes
#include "bitvector.h"
#include "sparsearray.h"

constexpr void ASSERT_EQUAL(auto a, auto b, std::string const& msg) {
    if (a != b) {
        std::cerr << msg << std::endl;
        std::exit(1);
    }
}


void testBitVector();
void testRank();
void testSelect();
void testSparseArray();

int main() {

    testBitVector();
    testRank();
    testSelect();
    testSparseArray();

}

void testBitVector() {
    using namespace bitvector;
    std::cout << "Testing BitVector...\t\t";

    const std::string INIT_STRING = "10001010000111";
    const uint32_t INIT_STRING_POPCOUNT = 6;

    /* allocation */
    BitVector bv1(16);
    BitVector bv2(100);
    BitVector bv3(INIT_STRING);
    BitVector bvSmall(10);

    /* initialization values and getters */
    for (size_t i = 0; i < bv1.size(); i += 1) {
        ASSERT_EQUAL(bv1.at(i), 0, "Value not initialized to zero.");
    }
    
    for (size_t i = 0; i < bv2.size(); i += 1) {
        ASSERT_EQUAL(bv2.at(i), 0, "Value not initialized to zero.");
    }

    for (size_t i = 0; i < bv3.size(); i += 1) {
        uint8_t val = INIT_STRING.at(i) - '0';
        ASSERT_EQUAL(bv3.at(i), val, "Value does not match string value.");
    }

    /* setters */
    auto counter = 0;
    for (size_t i = 0; i < bv2.size(); i += 1) {
        bv2.set(i, ((i%3)==0));
        if (i%3 == 0) counter++;
    }

    for (size_t i = 0; i < bv2.size(); i += 1) {
        bool val = ((i%3)==0);
        ASSERT_EQUAL(bv2.at(i), val, "Value does not match set value.");
    }

    bvSmall.set(1, true);
    bvSmall.set(5, true);
    bvSmall.set(9, true);
    
    /* popcount */
    auto sum = bv2.popcount();
    ASSERT_EQUAL(sum, bv2.size()/3+1, "Popcount invalid.");

    sum = bv3.popcount();
    ASSERT_EQUAL(sum, INIT_STRING_POPCOUNT, "Popcount invalid.");

    sum = bvSmall.popcount();
    ASSERT_EQUAL(sum, 3u, "Popcount invalid.");

    sum = bvSmall.popcount(0, 6);
    ASSERT_EQUAL(sum, 2u, "Popcount invalid.");

    /* use as packed int array */
    const std::vector<uint32_t> bitsPerElement {8u, 3u, 12u, 20u, 32u, 54u};
    const uint32_t numElements = 150;
    std::vector<uint64_t> elements(numElements);
    for (auto const& bpe : bitsPerElement) {
        std::random_device device;
        std::mt19937 rng(device());
        std::uniform_int_distribution<uint64_t> dist{0, (1u << bpe) - 1};
        
        std::generate(std::begin(elements), std::end(elements), [&dist,&rng](){ return dist(rng); });

        /* create bitevector */
        PackedVector pv(numElements, bpe);

        /* set value */
        for (uint32_t i = 0; i < elements.size(); i += 1) {
            pv.set(i, elements.at(i));
        }

        /* check values */
        for (uint32_t i = 0; i < elements.size(); i += 1) {
            ASSERT_EQUAL(pv.at(i), elements.at(i), "Packed integer invalid on " + std::to_string(bpe) + 
                                                " bits per element");
        }
    }

    std::cout << "Success\n";
}

void testRank() {
    using namespace bitvector;
    std::cout << "Testing RankSupport...\t\t";

    /* small example -- block size 2, superblock size 8 */
    const std::string EXAMPLE_STR = "1001011101001010";
    const BitVector bv(EXAMPLE_STR);
    RankSupport rank(bv);

    for (size_t i = 0; i < bv.size(); i += 1) {
        const auto val = rank(i);
        const auto prefix = EXAMPLE_STR.substr(0, i+1);
        const uint32_t expected = std::count_if(std::begin(prefix), std::end(prefix), [](auto c) { return c == '1'; });

        ASSERT_EQUAL(val, expected, "Incorrect rank calculated.");
    }
    //ASSERT_EQUAL(rank.overhead(), 320, "Incorrect overhead.");

    /* even smaller example */
    const std::string SMALL_STR = "0100010001";
    const BitVector bvSmall(SMALL_STR);
    RankSupport rankSmall(bvSmall);
    for (size_t i = 0; i < bvSmall.size(); i += 1) {
        const auto val = rankSmall(i);
        const auto prefix = SMALL_STR.substr(0, i+1);
        const uint32_t expected = std::count_if(std::begin(prefix), std::end(prefix), [](auto c) { return c == '1'; });

        ASSERT_EQUAL(val, expected, "Invalid rank calculated.");
    }

    /* longer examples */
    const std::vector<uint32_t> LENGTHS {10, 1024, 4096, 1000, 1001, 10057};
    for (auto const& len : LENGTHS) {
        const std::string BIT_STR = getRandomBinaryString(len);
        BitVector bvLong(BIT_STR);
        RankSupport rankLong(bvLong);

        /* save to file */
        rankLong.save("junk.ranksupport");

        for (size_t i = 0; i < bvLong.size(); i += 1) {
            auto val = rankLong(i);
            auto prefix = BIT_STR.substr(0, i+1);
            uint32_t expected = std::count_if(std::begin(prefix), std::end(prefix), [](auto c) { return c == '1'; });

            ASSERT_EQUAL(val, expected, "Incorrect rank calculated (length=" + std::to_string(len) + 
                                        ", index=" + std::to_string(i) + ").");
        }

        /* read from file and do again */
        rankLong.load("junk.ranksupport");
        for (size_t i = 0; i < bvLong.size(); i += 1) {
            auto val = rankLong(i);
            auto prefix = BIT_STR.substr(0, i+1);
            uint32_t expected = std::count_if(std::begin(prefix), std::end(prefix), [](auto c) { return c == '1'; });

            ASSERT_EQUAL(val, expected, "Incorrect rank calculated (length=" + std::to_string(len) + 
                                        ", index=" + std::to_string(i) + ") after file load.");
        }

    }

    std::remove("junk.ranksupport");

    std::cout << "Success\n";
}

 uint64_t naiveSelect(std::string const& s, char val, uint64_t count) noexcept {
    uint64_t occurrences = 0;

    size_t currentPosition = s.find(val, 0);
    if (currentPosition == std::string::npos) {
        return -1;
    }
    occurrences += 1;

    while (occurrences < count) {
        currentPosition += 1;
        currentPosition = s.find(val, currentPosition);

        if (currentPosition == std::string::npos) {
            return -1;
        }
        occurrences += 1;
    }

    return currentPosition;
}

void testSelect() {
    using namespace bitvector;
    std::cout << "Testing SelectSupport...\t";

    /* small example -- block size 2, superblock size 8 */
    const std::string EXAMPLE_STR = "1001011101001010";
    const uint64_t EXAMPLE_NUM_ONES = std::count(std::begin(EXAMPLE_STR), std::end(EXAMPLE_STR), '1');
    BitVector bv(EXAMPLE_STR);
    RankSupport rank(bv);
    SelectSupport select(rank);

    for (size_t i = 1; i <= EXAMPLE_NUM_ONES; i += 1) {
        const auto val = select(i);
        const auto expected = naiveSelect(EXAMPLE_STR, '1', i);

        ASSERT_EQUAL(val, expected, "Incorrect select calculated.");
    }

    /* longer examples */
    const std::vector<uint32_t> LENGTHS {10, 65, 1024, 4096, 1000, 1001, 10057};
    for (auto const& len : LENGTHS) {
        const std::string BIT_STR = getRandomBinaryString(len);
        const uint64_t NUM_ONES = std::count(std::begin(BIT_STR), std::end(BIT_STR), '1');
        BitVector bvLong(BIT_STR);
        RankSupport rankLong(bvLong);
        SelectSupport selectLong(rankLong);

        for (size_t i = 1; i <= NUM_ONES; i += 1) {
            const auto val = selectLong(i);
            const auto expected = naiveSelect(BIT_STR, '1', i);

            ASSERT_EQUAL(val, expected, "Incorrect select calculated (length=" + std::to_string(len) + 
                                        ", index=" + std::to_string(i) + ").");
        }
    }

    std::cout << "Success\n";
}

void testSparseArray() {
    std::cout << "Testing SparseArray...\t\t";

    /* simple test */
    {
        /* construction */
        sparse::SparseArray<std::string> array;
        
        /* creation */
        array.create(10);

        /* appending */
        array.append("foo", 1);
        array.append("bar", 5);
        array.append("baz", 9);

        /* get at rank */
        std::string tmp;
        auto ret = array.getAtRank(1, tmp);
        ASSERT_EQUAL(ret, true, "getAtRank invalid return.");
        ASSERT_EQUAL(tmp, "bar", "getAtRank invalid element value.");

        /* get at index */
        ret = array.getAtIndex(3, tmp);
        ASSERT_EQUAL(ret, false, "getAtIndex invalid return.");

        ret = array.getAtIndex(5, tmp);
        ASSERT_EQUAL(ret, true, "getAtIndex invalid return.");
        ASSERT_EQUAL(tmp, "bar", "getAtIndex invalid element value.");

        /* other */
        ASSERT_EQUAL(array.size(), 10u, "invalid size.");
        ASSERT_EQUAL(array.numElem(), 3u, "invalid number of elements.");
        ASSERT_EQUAL(array.numElemAt(5), 2u, "invalid numElemAt.");
        ASSERT_EQUAL(array.numElemAt(6), 2u, "invalid numElemAt.");
    }


    /* longer tests */
    const std::vector<uint32_t> LENGTHS {65, 1024, 4096, 1000, 1001, 10057};
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<uint64_t> distKey{1,10}, distVal{0ULL, ~0ULL};
    for (auto const& len : LENGTHS) {
        /* construction */
        sparse::SparseArray<uint64_t> array;

        /* creation */
        array.create(len);
        ASSERT_EQUAL(array.size(), len, "invalid size.");
        ASSERT_EQUAL(array.numElem(), 0u, "invalid number of elements.");

        /* append some elements */
        std::map<uint64_t, uint64_t> key;
        uint64_t indexCounter = distKey(rng);
        uint64_t insertedCounter = 0;
        while (indexCounter < len) {
            const auto insertedValue = distVal(rng);
            key.insert({indexCounter, insertedValue});
            array.append(insertedValue, indexCounter);

            indexCounter += distKey(rng);
            insertedCounter += 1;
        }
        ASSERT_EQUAL(array.numElem(), insertedCounter, "invalid number of elements.");

        /* write out the array */
        array.save("junk.sparsearray", true);

        /* check their values */
        int64_t rankCounter = 0;
        for (auto const& [index, value] : key) {
            uint64_t tmp;
            const auto valueAtIndex = array.getAtIndex(index, tmp);
            ASSERT_EQUAL(valueAtIndex, true, "invalid element at index return value.");
            ASSERT_EQUAL(tmp, value, "invalid element at index -- " + std::to_string(tmp) + " vs " + 
                std::to_string(value));
            
            const auto valueAtRank = array.getAtRank(rankCounter, tmp);
            ASSERT_EQUAL(valueAtRank, true, "invalid element at rank return value.");
            ASSERT_EQUAL(tmp, value, "invalid element at rank.");

            rankCounter += 1;
        }

        /* read array back in */
        array.load("junk.sparsearray");

        /* REDO tests after reading in file */
        rankCounter = 0;
        for (auto const& [index, value] : key) {
            uint64_t tmp;
            const auto valueAtIndex = array.getAtIndex(index, tmp);
            ASSERT_EQUAL(valueAtIndex, true, "invalid element at index return value (after load).");
            ASSERT_EQUAL(tmp, value, "invalid element at index -- " + std::to_string(tmp) + " vs " + 
                std::to_string(value) + " (after load).");
            
            const auto valueAtRank = array.getAtRank(rankCounter, tmp);
            ASSERT_EQUAL(valueAtRank, true, "invalid element at rank return value (after load).");
            ASSERT_EQUAL(tmp, value, "invalid element at rank (after load).");

            rankCounter += 1;
        }
    }

    std::remove("junk.sparsearray");

    std::cout << "Success\n";
}
