/*  Utility functions for BitVector, RankSupport, SelectSupport, and SparseArray 
    author: Daniel Nichols
    date: February 2022
*/
#pragma once
#include <cstdint>
#include <type_traits>

namespace utility {

/* allow turning off of bounds checking */
#if defined(NO_BOUNDS_CHECKING)
constexpr bool CHECK_BOUNDS = false;
#else
constexpr bool CHECK_BOUNDS = true;
#endif

/**
 * @brief Divides two integers and rounds up.
 * 
 * @param num division numerator
 * @param den division denominator
 * @return constexpr uint32_t ceil(num/den)
 */
constexpr uint32_t roundDivisionUp(uint32_t num, uint32_t den) noexcept {
    return num/den + (num%den == 0 ? 0 : 1);
}

/**
 * @brief Rounds an integer up to the next power of two.
 * modified from Stanford graphics bithacks page: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 * 
 * @param num input integer
 * @return constexpr uint32_t pow(ceil(log_2(num)), 2)
 */
constexpr uint32_t roundUpToPowerOf2(uint32_t num) noexcept {
    num -= 1;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    return num+1;
}

}   // end namespace utility

namespace serial {

/* forward declarations */
class ofstream;
class ifstream;

/**
 * @brief Concept resolves if T is a container as described in the standard "Container Named Requirements".
 * Concept definition is modified from https://stackoverflow.com/a/60450396/3769237
 */
template <typename T>
concept Container = requires(T a, const T b) {
    requires std::regular<T>;
    requires std::swappable<T>;
    requires std::destructible<typename T::value_type>;
    requires std::same_as<typename T::reference, typename T::value_type &>;
    requires std::same_as<typename T::const_reference, const typename T::value_type &>;
    requires std::forward_iterator<typename T::iterator>;
    requires std::forward_iterator<typename T::const_iterator>;
    requires std::signed_integral<typename T::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::iterator>::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::const_iterator>::difference_type>;
    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { b.begin() } -> std::same_as<typename T::const_iterator>;
    { b.end() } -> std::same_as<typename T::const_iterator>;
    { a.cbegin() } -> std::same_as<typename T::const_iterator>;
    { a.cend() } -> std::same_as<typename T::const_iterator>;
    { a.size() } -> std::same_as<typename T::size_type>;
    { a.max_size() } -> std::same_as<typename T::size_type>;
    { a.empty() } -> std::same_as<bool>;
};

/**
 * @brief Concept resolves if T implements a resize function. 
 */
template <typename T>
concept Resizable = requires(T a) {
    { a.resize(1u) } -> std::same_as<void>;
};

/**
 * @brief Concept resolves if T is trivial data type or container.
 * @note "Serializable" is a bit of a misnomer. If T is a container, then it is not necessarily serializable. It's
 *       sub-data type may not be serializable, but concepts don't allow recursive definitions. This is still useful 
 *       for the serialize/deserialize functions though.
 * @see std::is_trivial
 * @see Container 
 */
template <typename T>
concept Serializable = std::is_trivial<T>::value || Container<T>;

/**
 * @brief Serialize an objects bytes into an ofstream. If trivial (POD, bare struct with simple extant, etc...), 
 *        then this will just write out the data. If DataType is a container, then serialize will be recursively called
 *        on each value.
 * @see deserialize
 * 
 * @tparam DataType serializable
 * @param data data to serialize
 * @param outputStream location of resulting data
 */
template <Serializable DataType>
void serialize(DataType const& data, std::ofstream &outputStream) {

    if constexpr (Container<DataType>) {
        const auto size = data.size();
        outputStream.write(reinterpret_cast<char const*>(&size), sizeof(size));
        for (auto const& value : data) {    
            serialize(value, outputStream);
        }
    } else {
        outputStream.write(reinterpret_cast<char const*>(&data), sizeof(data));
    }
}


/**
 * @brief Deserialize bytes from an ifstream into data object. If trivial (POD, bare struct with simple extant, etc...), 
 *        then this will just read in the data. If DataType is a container, then deserialize will be recursively called
 *        on each value. Expects the format/ordering use by `serialize` for containers.
 * @see serialize
 * 
 * @tparam DataType serializable
 * @param data where to write data
 * @param inputStream location of incoming data
 */
template <Serializable DataType>
void deserialize(DataType &data, std::ifstream &inputStream) {
    if constexpr (Container<DataType>) {
        const auto currentSize = data.size();
        auto size = decltype(currentSize){};

        inputStream.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (currentSize != size) {
            if constexpr(Resizable<DataType>) {
                data.resize(size);
            } else {
                throw std::runtime_error("Container size mismatch during deserialization. Cannot recover.");
            }
        }

        for (auto &value : data) {
            deserialize(value, inputStream);
        }
    } else {
        inputStream.read(reinterpret_cast<char *>(&data), sizeof(data));
    }
}

}   // end namespace serialize
