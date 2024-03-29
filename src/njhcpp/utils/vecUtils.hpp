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


/**@brief Add container contents to a vector
 *
 * @tparam T the type of the vector
 * @tparam CON the other container type, e.g. set, unordered_set, etc
 * @param vec the vector to add to
 * @param container the contain to add content from
 */
template<typename T, typename CON>
void addConToVec(std::vector<T> & vec, const CON & container){
	std::copy(container.begin(), container.end(), std::back_inserter(vec));
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

/**@brief Convert vector to set
 *
 * @tparam T the template type
 * @param vec the vector to convert
 * @return a set of the vector contents
 */
template<typename T>
std::set<T> vecToSet(const std::vector<T> & vec){
	return std::set<T>(vec.begin(), vec.end());
}

/**@brief Convert unordered_set to set
 *
 * @tparam T the template type
 * @param uos the unordered_set to convert
 * @return a set of the unordered_set contents
 */
template<typename T>
std::set<T> uosetToSet(const std::unordered_set<T> & uos){
	return std::set<T>(uos.begin(), uos.end());
}

/**@brief Convert set to unordered_set
 *
 * @tparam T the template type
 * @param uos the set to convert
 * @return a unordered_set of the set contents
 */
template<typename T>
std::unordered_set<T> setToUOSet(const std::set<T> & s){
	return std::unordered_set<T>(s.begin(), s.end());
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

/**@brief Convert vector to unordered_set
 *
 * @tparam T the template type
 * @param vec the vector to convert
 * @return an unordered_set of the vector contents
 */
template<typename T>
std::unordered_set<T> vecToUOSet(const std::vector<T> & vec){
	return std::set<T>(vec.begin(), vec.end());
}

} //namespace njh

