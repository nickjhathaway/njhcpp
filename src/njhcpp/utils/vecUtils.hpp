#pragma once
/*
 * vecUtils.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: nickhathaway
 */


#include <vector>
#include <set>
#include <unordered_set>
#include <string>
#include <algorithm>

namespace njh {
/**@brief Get a sub-selection of a vector similar to std::string's substr
 *
 * @param vec the vector to get a subsection from
 * @param start the start of the subsection
 * @param size the size of the subsection
 * @return Return the subsection from start of size size
 * @todo might want to add a range bounds check
 */
template <typename T>
std::vector<T> getSubVector(const std::vector<T>& vec, uint32_t start,
                            uint32_t size) {
  std::vector<T> ret(vec.begin() + start, vec.begin() + size + start);
  return ret;
}

/**@brief Get a sub-selection of a vector similar to std::string's substr
 *
 * @param vec the vector to get a subsection from
 * @param start the start of the subsection
 * @return Return the subsection from start of size size
 * @todo might want to add a range bounds check
 */
template <typename T>
std::vector<T> getSubVector(const std::vector<T>& vec, uint32_t start) {
  std::vector<T> ret(vec.begin() + start, vec.end());
  return ret;
}

/**@brief Combine two vectors
 *
 * @param vec1 First vector
 * @param vec2 Second vector
 * @return A vector with a combination of the vectors
 */
template <class T>
const std::vector<T> catVecs(const std::vector<T>& vec1,
                                     const std::vector<T>& vec2) {
  std::vector<T> ans;
  ans.reserve(vec1.size() + vec2.size());
  ans.insert(ans.end(), vec1.begin(), vec1.end());
  ans.insert(ans.end(), vec2.begin(), vec2.end());
  return ans;
}


/**@brief Add values of a vector to a set
 *
 * @param vec the vector to add
 * @param s the set to add to
 */
template<typename T>
void addVecToSet(const std::vector<T> & vec, std::set<T> & s) {
	std::copy(vec.begin(), vec.end(), std::inserter(s, s.end()));
}

/**@brief Add values of a vector to a unordered_set;
 *
 * @param vec the vector to add
 * @param s the unordered_set to add to
 */
template<typename T>
void addVecToUOSet(const std::vector<T> & vec, std::unordered_set<T> & s) {
	std::copy(vec.begin(), vec.end(), std::inserter(s, s.end()));
}

} //namespace njh

