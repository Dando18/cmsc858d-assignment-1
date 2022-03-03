/*  Implementations SparseArray
    author: Daniel Nichols
    date: February 2022
*/
#pragma once

// stl includes
#include <vector>

// local includes
#include "bitvector.h"
#include "utilities.h"

namespace sparse {

/**
 * @brief SparseArray
 * 
 * @tparam T type to store within array. Can be any valid type. Compiling ::load and ::save will give errors if T is 
 *              not a trivial type or container of trivial types (or container of container of etc...).
 *              See std::is_trivial<> and utilities.h for definition of concepts.
 */
template<typename T>
class SparseArray {
    /**
     * @brief All saved SparseArray files should start with these 4 bytes.
     */
    constexpr static uint32_t FILE_MAGIC = 0xdeadbeef;

    public:

        /**
         * @brief Construct a new SparseArray object. Empty initially.
         * @see create
         * 
         */
        SparseArray() noexcept : bitvector_(8), rank_(bitvector_) {}

        /**
         * @brief initialize the array with size `size`
         * 
         * @param size size of the sparse array
         */
        void create(uint64_t size) noexcept {
            bitvector_ = bitvector::BitVector(size);
            rank_ = bitvector::RankSupport(bitvector_);
            values_.clear();
        }

        /**
         * @brief Add element to end of sparse array. Copy of `elem` is moved into array.
         * @throws std::invalid_argument if the position is already set.
         * @throws std::out_of_range if the position is out of bounds.
         * 
         * @param elem element to append
         * @param pos where to insert it
         */
        void append(T const& elem, uint64_t pos) {
            if constexpr (utility::CHECK_BOUNDS) {
                if (bitvector_.at(pos)) {
                    throw std::invalid_argument("SparseArray::append -- position " + std::to_string(pos) + 
                        " already set.");
                }
            }

            values_.push_back(elem);
            bitvector_.set(pos, 1);
            rank_.buildTables(pos);
        }

        /**
         * @brief return the rank-th element of the sparse array.
         * 
         * @param rank what rank element to retrieve
         * @param element receives reference to value in sparse array
         * @return true if rank >= the number of elements
         * @return false if rank < the number of elements
         */
        bool getAtRank(uint64_t rank, T &element) {
            if (rank < this->numElem()) {
                element = values_.at(rank);
                return true;
            }
            return false;
        }

        /**
         * @brief get the element of the sparse array at `index`.
         * @throws std::out_of_range if index is out of bounds.
         * 
         * @param index index of sparse array
         * @param element receives value at `index`
         * @return true if value was present
         * @return false if value was not present
         */
        bool getAtIndex(uint64_t index, T &element) {
            if (bitvector_.at(index)) {
                const auto arrayIndex = rank_(index);
                element = values_.at(arrayIndex-1); /* arrayIndex shouldn't be 0, since bv[index]=1 => rank(index)>=1*/
                return true;
            }
            return false;
        }

        /**
         * @brief Counts the number of elements up to index.
         * 
         * @param index 
         * @return uint64_t number of elements up to `index`
         */
        uint64_t numElemAt(uint64_t index) const {
            return rank_(index);
        }

        /**
         * @brief The size of the SparseArray. This is the total number of elements it can store.
         * 
         * @return uint64_t size of sparsearray
         */
        uint64_t size() const noexcept {
            return bitvector_.size();
        }

        /**
         * @brief The total number of elements currently in the SparseArray. 
         * 
         * @return uint64_t total elements.
         */
        uint64_t numElem() const noexcept {
            return values_.size();
        }

        /**
         * @brief Save SparseArray to file. 
         * @see load
         * @throws std::ios_base::failure If there is an error opening the file.
         * 
         * @param fname Filename of file to write to.
         * @param saveRankTables If true, then the ranktable data will be saved in the file. If false, then it is left
         *        out and `load` will regenerate it.
         */
        void save(std::string const& fname, bool saveRankTables=false) {
            std::ofstream outputStream(fname, std::ios::out | std::ios::binary);
            if (!outputStream) {
                throw std::ios_base::failure("SparseArray::save -- Could not open file \"" + fname + "\" to write.");
            }

            /* meta data */
            const uint32_t tmpMagic = SparseArray::FILE_MAGIC;
            const uint32_t tmpDataSize = sizeof(T);
            const uint32_t tmpSize = this->size();
            serial::serialize(tmpMagic, outputStream);
            serial::serialize(tmpDataSize, outputStream);
            serial::serialize(tmpSize, outputStream);

            /* write bits in bitvector */
            const uint32_t numBitvectorBytes = utility::roundDivisionUp(tmpSize, 8);
            outputStream.write(reinterpret_cast<char const*>(bitvector_.data()), numBitvectorBytes);

            /* write values */
            serial::serialize(values_, outputStream);

            /* save rank information */
            if (saveRankTables) {
                serial::serialize(rank_.superblocks_, outputStream);
                serial::serialize(rank_.blocks_, outputStream);
            }

            outputStream.close();
        }

        /**
         * @brief Loads a SparseVector from a file. Expects the format used in SparseVector::save.
         * @see save
         * @throws std::ios_base::failure if file not found, invalid, or some other file error.
         * 
         * @param fname Name of file to load.
         */
        void load(std::string const& fname) {
            std::ifstream inputStream(fname, std::ios::in | std::ios::binary);
            if (!inputStream) {
                throw std::ios_base::failure("SparseArray::load -- Could not open file \"" + fname + "\" to read.");
            }

            /* metadata */
            uint32_t tmpMagic, tmpDataSize, tmpSize;
            serial::deserialize(tmpMagic, inputStream);
            if (tmpMagic != SparseArray::FILE_MAGIC) {
                inputStream.close();
                throw std::ios_base::failure("SparseArray::load -- Invalid file format reading \"" + fname + "\".");
            }
            serial::deserialize(tmpDataSize, inputStream);
            if (tmpDataSize != sizeof(T)) {
                inputStream.close();
                throw std::ios_base::failure("SparseArray::load -- File \"" + fname + "\" saves different data type.");
            }
            serial::deserialize(tmpSize, inputStream);

            /* allocate data and read in bitvector */
            this->create(tmpSize);
            const uint32_t numBitvectorBytes = utility::roundDivisionUp(bitvector_.size(), 8);
            inputStream.read(reinterpret_cast<char*>(bitvector_.data()), numBitvectorBytes);

            /* read in array */
            serial::deserialize(values_, inputStream);

            /* read in or rebuild rank tables */
            if (inputStream.peek() == EOF) {
                rank_.buildTables();
            } else {
                serial::deserialize(rank_.superblocks_, inputStream);
                serial::deserialize(rank_.blocks_, inputStream);
            }

            inputStream.close();
        }

        /**
         * @brief number of bits this data structure uses
         * 
         * @return uint64_t number of bits used to store meta data
         */
        uint64_t overhead() const noexcept {
            return 8*sizeof(T)*values_.size() + rank_.overhead() + bitvector_.size();
        }

    private:
        bitvector::BitVector bitvector_;
        bitvector::RankSupport rank_;
        std::vector<T> values_;
};

} // end namespace sparse