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

}  //namepsace njh


