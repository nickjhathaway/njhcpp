#pragma once
/*
 * vecUtils.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: nickhathaway
 */


#include <vector>
#include <string>
#include <algorithm>

namespace bib {
/**@b Get a sub-selection of a vector similar to std::string's substr
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

/**@b Get a sub-selection of a vector similar to std::string's substr
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


} //namespace bib

