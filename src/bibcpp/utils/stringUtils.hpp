#pragma once
/*
 * stringUtils.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <string>
#include <sstream>
#include <iostream>
#include <iterator> //std::ostream_iterator

namespace bib {
/**@b Take a container and change it into a delimited string
 *
 * @param con Container of values
 * @param delim The delimiter to use for output
 * @return A delimited string of the values in con
 */
template <typename Container>
std::string conToStr(const Container& con,
                     const std::string& delim = " ") {
  if (con.empty()) {
    return "";
  }
  std::stringstream out;
  std::copy(con.begin(), con.end(),
       std::ostream_iterator<typename Container::value_type>(out, delim.c_str()));
  std::string ret = out.str();
  ret.erase(ret.size() - delim.size());
  return ret;
}

} // namesapce bib
