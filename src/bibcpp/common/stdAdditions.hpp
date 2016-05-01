#pragma once
/*
 * stdAdditions.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */




#include "bibcpp/common/stdIncludes.hpp"

namespace bib{


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
template <typename Input, typename UnaryFunction>
UnaryFunction for_each_pos(Input& container, const std::vector<uint64_t> & positions,UnaryFunction f) {
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


}  //namepsace bib


