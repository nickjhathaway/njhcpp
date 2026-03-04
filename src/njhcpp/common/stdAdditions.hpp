#pragma once
/*
 * stdAdditions.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick hathaway
 */




#include "njhcpp/common/stdIncludes.hpp"

namespace njh{


template<typename T>
void iota(std::vector<T>& vec, const T& value) {
	std::iota(vec.begin(), vec.end(), value);
}

template<typename Container, typename URNG>
void shuffle(Container & con, URNG && g) {
	std::shuffle(con.begin(), con.end(), g);
}

template<typename Container, typename UnaryPredicate>
typename std::iterator_traits<typename Container::const_iterator>::difference_type count_if(
		const Container & con, UnaryPredicate p) {
	return std::count_if(con.begin(), con.end(), p);
}

template<typename Container, typename UnaryPredicate>
typename std::iterator_traits<typename Container::iterator>::difference_type count_if(Container & con, UnaryPredicate p){
	return std::count_if(con.begin(), con.end(), p);
}

template <typename Input, typename UnaryFunction>
UnaryFunction for_each(Input& container, UnaryFunction f) {
  return std::for_each(container.begin(), container.end(), f);
}


/**@b non range safe position specific operations
 *
 * @param container the container to operator on
 * @param positions the positions to operator on
 * @param f the func to employ
 * @return the func used
 */
template <typename Input,typename POS, typename UnaryFunction>
UnaryFunction for_each_pos(Input& container, const std::vector<POS> & positions, UnaryFunction f) {
	for(auto pos : positions){
		f(container[pos]);
	}
  return f;
}

template <typename Input, typename UnaryFunction>
UnaryFunction for_each(const Input& container, UnaryFunction f) {
  return std::for_each(container.begin(), container.end(), f);
}

template <typename Container>
void sort(Container& con) {
  return std::sort(con.begin(), con.end());
}

template <typename Container, typename Compare>
void sort(Container& con, Compare comp) {
  return std::sort(con.begin(), con.end(), comp);
}

template <typename Container>
void reverse(Container& con) {
  std::reverse(con.begin(), con.end());
}


/**
 * @brief Take two sorted ranges and decompose them into unique input1, unique input2, and shared between inputs
 * @tparam Input1 The type of input1 sorted range (e.g. std::set, std::vector that's been sorted), can be pointer to a subset of sorted range
 * @tparam Input2 The type of input2 sorted range (e.g. std::set, std::vector that's been sorted), can be pointer to a subset of sorted range
 * @tparam Output1 inserter type for unique to input1
 * @tparam Output2 inserter type for unique to input2
 * @tparam Output3 inserter type for shared between inputs
 * @param first1 beginning of sorted range input 1
 * @param last1 ending of sorted range input 1
 * @param first2 beginning of sorted range input 2
 * @param last2 ending of sorted range input 2
 * @param result1 only in Input1
 * @param result2 only in Input2
 * @param result3 in both Inputs
 * @return the pointer to shared range
 */
template<class Input1, class Input2,
				class Output1, class Output2, class Output3>
Output3 decompose_sets(Input1 first1, Input1 last1,
											 Input2 first2, Input2 last2,
											 Output1 result1, Output2 result2,
											 Output3 result3) {
	//credit https://stackoverflow.com/questions/18164426/set-difference-and-set-intersection-simultaneously
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			*result1++ = *first1++;
		} else if (*first2 < *first1) {
			*result2++ = *first2++;
		} else {
			*result3++ = *first1++;
			++first2; // skip common value in set2
		}
	}
	std::copy(first1, last1, result1);
	std::copy(first2, last2, result2);
	return result3;
}


/**
 * @brief Result structure returned by set decomposition.
 *
 * Contains the decomposition of two sorted containers into:
 * - elements only in the first container
 * - elements only in the second container
 * - elements shared by both containers
 * - the union of both containers
 *
 * @tparam T Element type stored in the containers.
 */
template <typename T>
struct SetDecompositionResult {
    std::vector<T> only_in_first;   ///< Elements present only in the first container
    std::vector<T> only_in_second;  ///< Elements present only in the second container
    std::vector<T> shared;          ///< Elements present in both containers
    std::vector<T> all;             ///< Union of both containers
};


/**
 * @brief Decompose two sorted containers into unique and shared elements.
 *
 * This is a convenience wrapper around `decompose_sets()` that accepts
 * containers instead of iterator ranges. The inputs must already be sorted
 * according to `operator<`.
 *
 * The function validates that both containers are sorted and throws
 * `std::invalid_argument` if either container is not sorted.
 *
 * Internally, it calls `decompose_sets()` to compute:
 *
 * - Elements only in the first container
 * - Elements only in the second container
 * - Elements shared by both containers
 *
 * The union (`all`) is constructed from the three outputs.
 *
 * @tparam Container1 First container type
 * @tparam Container2 Second container type
 *
 * @param c1 First sorted container
 * @param c2 Second sorted container
 *
 * @return SetDecompositionResult containing decomposition results
 *
 * @throws std::invalid_argument if either container is not sorted
 *
 * @note Containers must contain comparable values using `operator<`.
 * @note Works with any container supporting `begin()` and `end()`.
 *
 * @example
 * std::vector<int> a{1,2,3,5};
 * std::vector<int> b{2,3,4};
 *
 * auto res = decompose_sets_container(a, b);
 *
 * // res.only_in_first  -> {1,5}
 * // res.only_in_second -> {4}
 * // res.shared         -> {2,3}
 * // res.all            -> {1,2,3,4,5}
 */
template <typename Container1, typename Container2>
auto decompose_sets_container(const Container1& c1, const Container2& c2)
{
    using T = typename Container1::value_type;

    static_assert(
        std::is_same<T, typename Container2::value_type>::value,
        "Containers must have the same value_type"
    );

    if (!std::is_sorted(c1.begin(), c1.end()))
        throw std::invalid_argument("First container must be sorted");

    if (!std::is_sorted(c2.begin(), c2.end()))
        throw std::invalid_argument("Second container must be sorted");

    SetDecompositionResult<T> result;

    decompose_sets(
        c1.begin(), c1.end(),
        c2.begin(), c2.end(),
        std::back_inserter(result.only_in_first),
        std::back_inserter(result.only_in_second),
        std::back_inserter(result.shared)
    );

    // Construct union (all)
    result.all.reserve(
        result.only_in_first.size() +
        result.only_in_second.size() +
        result.shared.size()
    );

    std::merge(
        c1.begin(), c1.end(),
        c2.begin(), c2.end(),
        std::back_inserter(result.all)
    );

    result.all.erase(
        std::unique(result.all.begin(), result.all.end()),
        result.all.end()
    );

    return result;
}


}  //namepsace njh


