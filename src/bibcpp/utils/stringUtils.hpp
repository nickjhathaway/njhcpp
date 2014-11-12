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

/**@b Check to see if a string contains substrings of interest
 *
 * @param str The string to search
 * @param contains The substrings to search for
 * @return True if string contains all substrings and false else wise
 */
inline bool checkForSubStrs(const std::string & str,
		const std::vector<std::string> & contains){
	for(const auto & s : contains){
		if(str.find(s) == std::string::npos){
			return false;
		}
	}
	return true;
}

/**@b Pad a number so that it sorts well with the rest of the numbers
 *
 * @param num The number to pad
 * @param highestNumber The highest number from the collectio that num belongs to, to determine how much to pad
 * @return
 */
template <typename T>
std::string leftPadNumStr(T num, T highestNumber = 10) {
	static_assert(std::is_unsigned<T>::value,
	                  "Num has to be unsigned");
  std::stringstream ret;
  if (num == 0) {
    ret << std::string(log10(highestNumber), '0');
  } else {
    ret << std::string((static_cast<int32_t>(log10(highestNumber)) - static_cast<int32_t>(log10(num))), '0');
  }
  ret << num;
  return ret.str();
}

} // namesapce bib
