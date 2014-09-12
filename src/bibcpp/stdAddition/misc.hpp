#pragma once
/*
 * misc.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <string>


namespace estd {
/**@b simply aesthetic, to make call to is_arithmetic look nicer
 *
 * @return Returns true if T is of type arithmetic
 */
template<typename T>
constexpr bool isArithmetic(){
	return std::is_arithmetic<T>::value;
}


/**@b for determining if T is of type char* or const char* to declare that is a string
 *
 */
template<typename T>
struct is_string : public std::integral_constant<bool, std::is_same<char*, typename std::decay<T>::type>::value
|| std::is_same<const char*, typename std::decay<T>::type>::value> {};

/**@b Declared so that is_string will return true for std::string along with char* and const char*
 *
 */
template<>
struct is_string<std::string> : std::true_type {};


/**@b Nicer to_string than the std:: because it won't have trailing zeros for doubles
 *
 * @param e The element to change into string, needs to have operator << declared
 * @return Return what ever operator << puts into the stream
 */

/**@b simply aesthetic, to make call to is_string look nicer
 *
 * @return Returns true if T is of type arithmetic
 */
template<typename T>
constexpr bool isString(){
	return is_string<T>::value;
}

template <typename T>
std::string to_string(T e) {
  std::stringstream ss;
  ss << e;
  return ss.str();
}

}  // namespace estd
