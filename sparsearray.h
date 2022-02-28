/*  Implementations SparseArray
    author: Daniel Nichols
    date: February 2022
*/
#pragma once

// stl includes
#include <vector>

// local includes
#include "bitvector.h"

namespace sparse {

#if defined(NO_BOUNDS_CHECKING)
constexpr bool CHECK_BOUNDS = false;
#else
constexpr bool CHECK_BOUNDS = true;
#endif

/**
 * @brief SparseArray
 * 
 * @tparam T type to store within array
 */
template<typename T>
class SparseArray {
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
            if constexpr (CHECK_BOUNDS) {
                if (bitvector_.at(pos)) {
                    throw std::invalid_argument("SparseArray::append -- position " + std::to_string(pos) + 
                        " already set.");
                }
            }

            values_.push_back(elem);
            bitvector_.set(pos, 1);
            rank_.buildTables();
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

        void save(std::string const& fname) {

        }

        void load(std::string const& fname) {

        }

    private:
        bitvector::BitVector bitvector_;
        bitvector::RankSupport rank_;
        std::vector<T> values_;
};

} // end namespace sparse