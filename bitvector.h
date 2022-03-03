/*  Implementations of BitVector, RankSupport, SelectSupport.
    author: Daniel Nichols
    date: February 2022
*/
#pragma once

// stl includes
#include <bit>
#include <cmath>
#include <cstdint>
#include <exception>
#include <fstream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

// local includes
#include "utilities.h"

/* forward declarations */
namespace sparse { template<typename T> class SparseArray; }    /* so RankSupport and SelectSupport can friend it */

namespace bitvector {

/**
 * @brief BitVector class for storing bit arrays in a compact format.
 */
class BitVector {
    public:
        /**
         * @brief Construct a new BitVector object. Creates room for `size` bits.
         * 
         * @param size number of bits.
         */
        BitVector(uint64_t size) noexcept : size_(size), 
            data_(std::make_unique<uint8_t[]>(utility::roundDivisionUp(size_, 8))) {}

        /**
         * @brief Construct a new BitVector object from an input binary string. Assumes the string
         * holds binary in big-endian format.
         * 
         * @param value binary string
         */
        BitVector(std::string const& value) noexcept : BitVector(value.size()) {
            for (size_t i = 0; i < value.size(); i += 8) {
                std::string byteStr = value.substr(i, 8);
                std::reverse(std::begin(byteStr), std::end(byteStr));   // handle endianness
                data_[i >> 3] = static_cast<uint8_t>( std::stoul(byteStr, nullptr, 2) );
            }
        }

        /**
         * @brief Gets the `index`-th element of the bitvector. No bounds checking is done, so an invalid `index`
         * may raise segfault or return junk value.
         * 
         * @param index index to retrieve
         * @return true bit `index` is set
         * @return false bit `index` is not set 
         */
        bool operator[](uint64_t index) const noexcept {
            const uint64_t arrayIndex = index >> 3;
            const uint8_t bitIndex = index & 7;
            return data_[arrayIndex] & (1 << bitIndex);
        }

        /**
         * @brief Gets the `index`-th element of the bitvector.
         * 
         * @throws std::out_of_range If index is out of range.
         * 
         * @param index index to retrieve
         * @return true bit `index` is set
         * @return false bit `index` is not set
         */
        bool at(uint64_t index) const {
            checkIndexBounds(index);
            return operator[](index);
        }

        /**
         * @brief Sets the `index`-th bit of the bitvector. 
         * 
         * @throws std::out_of_range If index is out of range.
         * 
         * @param index index to set
         * @param bit set index to 0 or 1
         */
        void set(uint64_t index, bool bit) {
            checkIndexBounds(index);
            
            const uint64_t arrayIndex = index >> 3;
            const uint8_t bitIndex = index & 7;
            data_[arrayIndex] = (data_[arrayIndex] & ~(1 << bitIndex)) | (bit << bitIndex);
        }

        /**
         * @brief Popcount of the entire bitvector i.e. the total number of 1s.
         * 
         * @return uint32_t Total number of 1s in bitvector
         */
        uint64_t popcount() const noexcept {
            return std::transform_reduce(data_.get(), data_.get() + utility::roundDivisionUp(size_, 8), 0, 
                    std::plus<uint64_t>(), [](auto const& a) -> uint64_t { return std::popcount(a); });
        }

        /**
         * @brief Popcount of the byte that `index` lies in.
         * 
         * @throws std::out_of_range If index is out of range.
         * 
         * @param index index into bitvector
         * @return uint32_t popcount of entire byte index is in
         */
        uint8_t popcount(uint64_t index) const {
            checkIndexBounds(index);
            return std::popcount(data_[index >> 3]);
        }

        /**
         * @brief Returns the popcount of a range in bitvector. Range must be <= 32 bits.
         * 
         * Computed by loading bit range into 32 bit integer, clearing unnecessary bits with shifts, and calling
         * machine popcount. This should be 1 load and approximately 3 bit operations and, thus, is O(1) in `len`.
         * 
         * Note: uses std::popcount to compute popcount. The STL standard doesn't require this to use popcnt 
         * instruction, but gcc, clang, and msvc all implement it using the `__builtin_popcount()` intrinsic 
         * plus some other optimizations.
         * 
         * @throws std::out_of_range If start is out of bounds.
         * @throws std::invalid_argument If len is greater than 32 or start+len overflows a 32 bit int
         * 
         * @param start first bit to include in popcount
         * @param len number of bits in total to use
         * @return uint32_t the number of ones in range [start, start+len)
         */
        uint32_t popcount(uint64_t start, uint32_t len) const {
            checkIndexBounds(start);
            if constexpr (utility::CHECK_BOUNDS) {
                if ((len > 32) || (start+len < start)) {
                    /* last condition checks for overflow */
                    throw std::invalid_argument("Cannot do machine popcount.");
                }
            }

            uint32_t val = *reinterpret_cast<uint32_t*>(&data_[start >> 3]);
            val >>= start & 7;
            val <<= 32-len;

            return std::popcount(val);
        }

        uint8_t *data() noexcept {
            return data_.get();
        }

        uint8_t const* data() const noexcept {
            return data_.get();
        }

        /**
         * @brief The number of bits in this bitvector.
         * 
         * @return constexpr uint32_t number of bits.
         */
        uint64_t size() const noexcept {
            return size_;
        }

        /**
         * @brief Streams each bit as a bool into `stream`
         * 
         * @param stream output stream
         * @param bitvector bitvector to stream
         * @return std::ostream& the ostream with bits streamed into it
         */
        friend std::ostream& operator<<(std::ostream& stream, BitVector const& bitvector) {
            for (size_t i = 0; i < bitvector.size_; i += 1) {
                stream << bitvector[i];
            }
            return stream;
        }

    private:
        uint64_t size_;
        std::unique_ptr<uint8_t[]> data_;

        /**
         * @brief Checks to see if `index` is in-bounds for bitvector.
         * 
         * @throws std::out_of_range If index is out of range.
         * @param index 
         */
        inline void checkIndexBounds(uint64_t index) const {
            if constexpr (utility::CHECK_BOUNDS) {
                if (index >= this->size_) {
                    throw std::out_of_range("Invalid index " + std::to_string(index) + " for bitvector with size " + 
                                            std::to_string(this->size_) + ".");
                }
            }
        }
};

/**
 * @brief Utility to generate a random string of 1s and 0s.
 * 
 * @param bits Number of bits (i.e. characters) in string
 * @return std::string the random string object
 */
std::string getRandomBinaryString(size_t bits) noexcept {
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<uint8_t> dist{'0', '1'};
    
    std::string tmpString(bits, '0');
    for (auto& c : tmpString) {
        c = dist(rng);
    }
    return tmpString;
}

/**
 * @brief Return a random bitvector with size `bits`.
 * 
 * @param bits number of bits in bitvector
 * @return BitVector the random bitvector object
 */
BitVector getRandomBitVector(size_t bits) noexcept {
    return BitVector(getRandomBinaryString(bits));
}


/**
 * @brief RankSupport class. Implements ability to compute rank of bitvector in constant time.
 */
class RankSupport {
    /**
     * @brief All RankSupport files should start with these 4 bytes.
     */
    constexpr static uint32_t FILE_MAGIC = 0xfeedbeef;

    public:
        /**
         * @brief Construct a new RankSupport object around `bitvector`. Builds ancillary data tables on construction.
         * 
         * @param bitvector input BitVector
         */
        RankSupport(BitVector const& bitvector) : bitvector_(bitvector), 
            superblockSize_(std::pow(std::log2(utility::roundUpToPowerOf2(bitvector.size())), 2) / 2), 
            blockSize_(std::log2(utility::roundUpToPowerOf2(bitvector.size())) / 2),
            superblocks_(utility::roundDivisionUp(bitvector.size(), superblockSize_), 0),
            blocks_(utility::roundDivisionUp(bitvector.size(), blockSize_), 0) {
            /* construct tables here */

            this->buildTables();
        }

        /**
         * @brief Builds superblock and block level tables based on underlying bitvector's data.
         * @throws std::out_of_range if startingIndex is larger than the bitvector size.
         * 
         * @param startingIndex index to start building table from
         */
        void buildTables(uint64_t startingIndex = 0) {
            if constexpr (utility::CHECK_BOUNDS) {
                if (startingIndex > this->size()) {
                    throw std::out_of_range("RankSupport::buildTables -- startingIndex " + 
                            std::to_string(startingIndex) + " is out of range for bitvector.");
                }
            }
            auto const& bv = bitvector_.get();

            /* round startingIndex down to superblock */
            startingIndex = (startingIndex / superblockSize_) * (superblockSize_);

            uint32_t superblockSum = superblocks_.at(startingIndex / superblockSize_);
            uint32_t blockSum = blocks_.at(startingIndex / blockSize_);
            for (size_t i = startingIndex; i < bv.size(); i += 1) {
                if (i % superblockSize_ == 0) {
                    superblocks_.at(i/superblockSize_) = superblockSum;
                    blockSum = 0;
                }
                if (i % blockSize_ == 0) {
                    blocks_.at(i/blockSize_) = blockSum;
                }
                blockSum += bv[i];
                superblockSum += bv[i];
            }
            totalOnes_ = superblockSum;
        }

        /**
         * @brief The number of 1 bits in range 0...i.
         * @see rank1
         * 
         * @throws std::invalid_argument If i is >= the number of bits in the bitvector.
         * 
         * @param i 
         * @return uint64_t 
         */
        uint64_t operator()(uint64_t i) const {
            return rank1(i);
        }

        /**
         * @brief The number of 1 bits in range 0...i.
         * 
         * @throws std::out_of_range If i is >= the number of bits in the bitvector.
         * 
         * @param i
         * @return uint64_t 
         */
        uint64_t rank1(uint64_t i) const {
            if constexpr (utility::CHECK_BOUNDS) {
                if (i >= this->size()) {
                    throw std::out_of_range("RankSupport::rank1 - " + std::to_string(i) + 
                        "-th bit is out of bounds for bitvector of size " + std::to_string(this->size()) + ".");
                }
            }

            auto const& bv = bitvector_.get();
            const uint64_t blockCount = bv.popcount((i/blockSize_)*blockSize_, (i % (blockSize_)) + 1);
            return superblocks_.at(i/superblockSize_) + blocks_.at(i/blockSize_) + blockCount;
        }

        /**
         * @brief Return the overhead in bits.
         * 
         * @return uint64_t bits overhead.
         */
        uint64_t overhead() const noexcept {
            return  8*sizeof(decltype(superblocks_)::value_type)*superblocks_.size() + 
                    8*sizeof(decltype(blocks_)::value_type)*blocks_.size();
        }

        /**
         * @brief Loads saved RankSupport data from file `fname`. Data must be in format written by `save` function.
         * @see save
         * 
         * @param fname input filename
         */
        void load(std::string const& fname) {
            std::ifstream inputStream(fname, std::ios::in | std::ios::binary);
            if (!inputStream) {
                throw std::ios_base::failure("Could not open file \"" + fname + "\" to read.");
            }

            /* meta data */
            uint32_t magicTmp;
            serial::deserialize(magicTmp, inputStream);
            if (magicTmp != FILE_MAGIC) {
                inputStream.close();
                throw std::domain_error("Invalid file magic for file \"" + fname + "\".");
            }

            uint32_t superblockSize, blockSize;
            serial::deserialize(superblockSize, inputStream);
            serial::deserialize(blockSize, inputStream);
            this->superblockSize_ = superblockSize;
            this->blockSize_ = blockSize;

            /* read rest of data */
            serial::deserialize(superblocks_, inputStream);
            serial::deserialize(blocks_, inputStream);

            /* cleanup */
            inputStream.close();
        }

        /**
         * @brief Saves RankSupport data to file `fname`.
         * @see load
         * 
         * @param fname output filename
         */
        void save(std::string const& fname) const {
            std::ofstream outputStream(fname, std::ios::out | std::ios::binary);
            if (!outputStream) {
                throw std::ios_base::failure("Could not open file \"" + fname + "\" to write.");
            }

            /* write meta data */
            const uint32_t magicTmp = FILE_MAGIC;
            serial::serialize(magicTmp, outputStream);
            serial::serialize(superblockSize_, outputStream);
            serial::serialize(blockSize_, outputStream);

            /* write superblocks and blocks */
            serial::serialize(superblocks_, outputStream);
            serial::serialize(blocks_, outputStream);

            /* cleanup */
            outputStream.close();
        }

        /**
         * @brief Returns the size of the underlying bitvector.
         * 
         * @return uint32_t size of underlying bitvector
         */
        uint64_t size() const noexcept {
            return bitvector_.get().size();
        }

        /**
         * @brief Returns the total number of 1s in the bitvector in constant time.
         * 
         * @return uint64_t total number of 1s.
         */
        uint64_t totalOnes() const noexcept {
            return totalOnes_;
        }

        friend class SelectSupport;
        template <typename> friend class ::sparse::SparseArray;

    private:
        std::reference_wrapper<const BitVector> bitvector_;
        uint32_t superblockSize_, blockSize_;
        std::vector<uint32_t> superblocks_, blocks_;
        uint64_t totalOnes_;
};


/**
 * @brief SelectSupport class. Implements routines for doing selection on a bitvector.
 */
class SelectSupport {
    public:
        /**
         * @brief Construct a new SelectSupport object. 
         * 
         * @param rank RankSupport object
         */
        SelectSupport(RankSupport const& rank) noexcept : rank_(rank) {}

        /**
         * @brief The location of the i-th 1 in the bitvector. 
         * @see select1
         * 
         * @throws std::invalid_argument If i is greater than the total number of ones.
         * @throws std::domain_error If no answer is found (something is really wrong here).
         * 
         * @param i number of ones
         * @return uint64_t index of i-th 1
         */
        uint64_t operator()(uint64_t i) const {
            return select1(i);
        }

        /**
         * @brief The location of the i-th 1 in the bitvector. 
         * 
         * @throws std::invalid_argument If i is greater than the total number of ones or is zero.
         * @throws std::domain_error If no answer is found (something is really wrong here).
         * 
         * @param i number of ones
         * @return uint64_t index of i-th 1
         */
        uint64_t select1(uint64_t i) const {

            auto const& rank = rank_.get();

            if constexpr (utility::CHECK_BOUNDS) {
                if (i > rank.totalOnes()) {
                    throw std::invalid_argument("SelectSupport::select1 - Cannot select " + std::to_string(i) + 
                        "-th 1 in bitvector with " + std::to_string(rank.totalOnes()) + " 1s.");
                }
                if (i == 0) {
                    throw std::invalid_argument("SelectSupport::select1 - 0-th 1 is ill-defined. Use 1-indexing.");
                }
            }

            auto const& bv = rank.bitvector_.get();
            uint32_t lower = 0, upper = rank.size();
            while (lower <= upper) {
                const uint32_t mid = (lower + upper) / 2;
                const auto valueAtMid = rank(mid);

                if (valueAtMid < i) {
                    lower = mid + 1;
                } else if ((valueAtMid > i) || (bv.at(mid) != 1)) {
                    upper = mid - 1;
                } else {
                    return mid;
                }
            }

            throw std::runtime_error("Unexpected error finding " + std::to_string(i) + "-th 1.");
        }

        /**
         * @brief Returns the overhead in bits of RankSelect.
         * 
         * @return uint64_t number of bits
         */
        uint64_t overhead() const noexcept {
            return 0;
        }

        /**
         * @brief Loads SelectSupport data to `fname`. Must be in format from `save`.
         * @see save
         * 
         * @param fname input filename
         */
        void load(std::string const& fname) {

        }

        /**
         * @brief Saves SelectSupport data to `fname`.
         * @see load
         * 
         * @param fname output filename
         */
        void save(std::string const& fname) const {

        }
    
    private:
        std::reference_wrapper<const RankSupport> rank_;
};

}   // end namespace bitvector