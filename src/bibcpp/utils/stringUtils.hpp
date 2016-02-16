#pragma once
/*
 * stringUtils.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <string>
#include <sstream> //std::stringstream
#include <iostream>
#include <iterator> //std::ostream_iterator
#include <cmath> //round()
#include <iomanip> //std::setFill(), std::setw()
#include <regex>
#include "bibcpp/common.h" //estd::to_string
#include "bibcpp/jsonUtils/jsonUtils.hpp" //included here so that most files will have json


namespace bib {

/**@brief Strip the right side of a string of all characters matching c
 *
 * @param str The string to strip
 * @param c The character to strip from str
 */
inline void rstrip(std::string & str, char c){
	size_t pos = str.size();
	while (pos != 0 && str[pos - 1] == c){
		--pos;
	}
	if(pos != str.size()){
		str.erase(str.begin() + pos, str.end());
	}
}

/**@brief Strip the right side of a string of all characters matching c, return result
 *
 * @param str The string to strip
 * @param c The character to strip from str
 * @return A string with c stripped from str
 */
inline std::string rstripRet(std::string str, char c){
	rstrip(str,c);
	return str;
}

/**@brief Strip the left side of a string of all characters matching c
 *
 * @param str The string to strip
 * @param c The character to strip from str
 */
inline void lstrip(std::string & str, char c){
	size_t pos = 0;
	if(!str.empty()){
		if(str[pos] == c){
			while(pos != str.size() - 1 && str[pos + 1] == c){
				++pos;
			}
			str.erase(str.begin(), str.begin() + pos + 1);
		}
	}
}

/**@brief Strip the left side of a string of all characters matching c, return result
 *
 * @param str The string to strip
 * @param c The character to strip from str
 * @return A string with c stripped from str
 */
inline std::string lstripRet(std::string str, char c){
	lstrip(str,c);
	return str;
}

/**@brief Take a container and change it into a delimited string
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

/**@brief Check to see if a string contains substrings of interest
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

/**@brief Check a string for several regex patterns
 *
 * @param str The string to check
 * @param contains The regex patterns to search for
 * @return true if str contains all the regex patterns in contains
 */
inline bool checkForPats(const std::string & str,
		const std::vector<std::regex> & contains){
	for(const auto & p : contains){
		if(!std::regex_match(str,p)){
			return false;
		}
	}
	return true;
}

/**@brief Check a string for several regex patterns and return false if the string contains any of them
 *
 * @param str The string to check
 * @param contains The regex patterns to search for
 * @return true if str contains any of the regex patterns are contained in the string
 */
inline bool checkForPatsExclude(const std::string & str,
		const std::vector<std::regex> & exclude){
	for(const auto & p : exclude){
		if(std::regex_match(str,p)){
			return false;
		}
	}
	return true;
}



/**@brief Pad a number so that it sorts well with the rest of the numbers
 *
 * @param num The number to pad
 * @param highestNumber The highest number from the collectio that num belongs to, to determine how much to pad
 * @return
 */
template <typename T>
std::string leftPadNumStr(T num, T highestNumber = 10) {
	//static_assert(std::is_unsigned<T>::value,
	  //                "Num has to be unsigned");
  std::stringstream ret;
  if (num == 0) {
    ret << std::string(log10(highestNumber), '0');
  } else {
    ret << std::string((static_cast<int32_t>(log10(highestNumber)) - static_cast<int32_t>(log10(num))), '0');
  }
  ret << num;
  return ret.str();
}

/**@brief return a vector of strings by spiting a string on a delimiter
 *
 * @param str The string to split
 * @param delim The delimiter to split on
 * @param addEmptyToEnd If a delimiter is found at the very end of the str whether to end an empty string to the ret
 * @return A vector of strings
 */
inline std::vector<std::string> tokenizeString(const std::string& str,
                                               const std::string& delim,
                                               bool addEmptyToEnd = false) {
  std::vector<std::string> ret;
  if ("whitespace" == delim) {
    std::stringstream ss(str);
    while (!ss.eof()) {
      std::string tempStr;
      ss >> tempStr;
      ret.emplace_back(tempStr);
    }
  } else {
    if (str.find(delim) == std::string::npos) {
      ret.push_back(str);
    } else {
      std::size_t pos = str.find(delim, 0);
      std::size_t oldPos = -delim.size();
      while (pos != std::string::npos) {
        ret.emplace_back(
            str.substr(oldPos + delim.size(), pos - oldPos - delim.size()));
        oldPos = pos;
        pos = str.find(delim, pos + 1);
      }
      if (oldPos + delim.size() == str.size()) {
        if (addEmptyToEnd) {
          ret.emplace_back("");
        }
      } else {
        ret.emplace_back(str.substr(oldPos + delim.size(), str.size() - 1));
      }
    }
  }
  return ret;
}

/**@brief Test to see if string contains all digits
 *
 * @param str The string to test
 * @return Return true if all chars in str is a digit
 * @todo also check for negative sign and for e (for scientific notattion)
 */
inline bool strAllDigits(const std::string& str) {
  for (const auto& c : str) {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

/**@brief Just a convience to check to see if a str contains a substring
 *
 * @param str the string to check
 * @param subString the substring to check for
 * @return True if str contains subString
 */
inline bool containsSubString(const std::string& str,
                              const std::string& subString) {
  return (str.find(subString) != std::string::npos);
}



/**@brief Convertor for bool to true or false str
 *
 * @param convert Bool to convert
 * @return true or false
 */
inline std::string boolToStr(bool convert) {
  if (convert) {
    return "true";
  } else {
    return "false";
  }
}


/**@brief Convert a string to all upper case letters
 *
 * @param str String to convert
 */
inline void strToUpper(std::string& str) {
  for (auto& c : str) {
    c = toupper(c);
  }
}
/**@brief Convert a string to all lower case letters
 *
 * @param str String to convert
 */
inline void strToLower(std::string& str) {
  for (auto& c : str) {
    c = tolower(c);
  }
}

/**@brief Convert a string to all upper case letters, leave original alone
 *
 * @param str String to convert
 * @return str but with all upper case letters
 */
inline std::string strToUpperRet(std::string str) {
  strToUpper(str);
  return str;
}

/**@brief  Convert a string to all lower case letters, leave original alone
 *
 * @param str String to convert
 * @return str but with all lower case letters
 */
inline std::string strToLowerRet(std::string str) {
  strToLower(str);
  return str;
}

/**@brief Convert a vector of strings to all lower case letters
 *
 * @param vec Vector to convert
 */
inline void strVecToLower(std::vector<std::string>& vec) {
  for (auto& v : vec) {
    strToLower(v);
  }
}

/**@brief Replace a substring in a string with a new substring
 *
 * @param theString The string to do the replacement on
 * @param toBeReplaced The substring to be replaced
 * @param replacement The replacement for the substring
 * @return A new string with replacement of substring done
 */
inline std::string replaceString(std::string theString,
                                 const std::string& toBeReplaced,
                                 const std::string& replacement) {
  size_t spaceSize = toBeReplaced.size();
  size_t currPos = theString.find(toBeReplaced);
  while (currPos != std::string::npos) {
    theString.replace(currPos, spaceSize, replacement);
    currPos = theString.find(toBeReplaced, currPos + replacement.size());
  }
  return theString;
}

/**@brief convert int to a string hex string
 *
 * @param i the int to convert
 * @return A hexstring
 */
inline std::string intToHex(int32_t i) {
  std::stringstream stream;
  // stream << "0x";
  stream << std::setfill('0') << std::setw(2) << std::hex << i;
  return stream.str();
}

/**@brief convert a hexstring to a int
 *
 * @param hString The hextstring to convert
 * @return the int version of the hexstring
 */
inline uint32_t hexToInt(const std::string& hString) {
  std::stringstream stream(hString);
  uint32_t ans;
  stream >> std::hex >> ans;
  return ans;
}


/**@brief trim beginning whitespace
 *
 * @param s the string from which to trim the whitespace
 * @return A reference to the trimmed white space string so it can be chained with other trimming options
 */
inline std::string &ltrim(std::string &s) {
    // from http://stackoverflow.com/a/217605
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

/**@brief trim end whitespace
 *
 * @param s the string from which to trim the whitespace
 * @return A reference to the trimmed white space string so it can be chained with other trimming options
 */
inline std::string &rtrim(std::string &s) {
    // from http://stackoverflow.com/a/217605
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

/**@brief trim both beginning and end whitepsace
 *
 * @param s the string from which to trim the whitespace
 * @return A reference to the trimmed white space string so it can be chained with other trimming options
 */
inline std::string &trim(std::string& s) {
    // from http://stackoverflow.com/a/217605
    return ltrim(rtrim(s));
}

template <typename CON, typename FUN>
std::string longestToString(const CON& con, FUN f){
    auto t = [&f](auto& e){ return estd::to_string(f(e)); };
    auto longest = std::max_element(con.begin(), con.end(), [&t](const auto& a, const auto& b){
            return t(a).size() < t(b).size();
        });
    return t(*longest);
}

template <typename CON, typename FUN>
uint32_t paddingWidth(const CON& con, FUN f){
    return longestToString(con, f).size();
}

/**@brief Print out the contents of a map with column adjusted, should be a simple key,value map where both the key and value can be converted to strings and
 * have the output operator << defined
 *
 * @param theMap The map to print
 * @param out The stream to print o
 */
template <typename MAP>
void mapOutColAdjust(const MAP& theMap,
                         std::ostream& out,
												 const std::string & middleSep = "\t",
												 uint32_t keyMaxLen = 0,
												 uint32_t valueMaxLen = 0) {
	if(0 == keyMaxLen){
		keyMaxLen = paddingWidth(theMap, [](const auto & kv){return kv.first;});
	}
	if(0 == valueMaxLen){
		valueMaxLen = paddingWidth(theMap, [](const auto & kv){return kv.second;});
	}

  for (const auto& mValue : theMap) {
    out << std::right << std::setw(keyMaxLen) << mValue.first << middleSep
    		<< std::left << std::setw(valueMaxLen) << mValue.second << "\n";
  }
}

template<typename MAP, typename KEYFUNC, typename VALUEFUNC>
void compareColPaddings(const MAP& theMap,
		std::pair<uint32_t, uint32_t> & paddings, KEYFUNC kFunc, VALUEFUNC vFunc) {
	uint32_t keyMaxLen = paddingWidth(theMap, kFunc);
	uint32_t valueMaxLen = paddingWidth(theMap, vFunc);
	if (keyMaxLen > paddings.first) {
		paddings.first = keyMaxLen;
	}
	if (valueMaxLen > paddings.first) {
		paddings.second = keyMaxLen;
	}
}

inline bool endsWith(const std::string& a, const std::string& b) {
  // http://stackoverflow.com/a/874160
  if (a.size() >= b.size()) {
    return (0 == a.compare(a.size() - b.size(), b.size(), b));
  }
  return false;
}

inline bool beginsWith(const std::string& str, const std::string& target) {
  if (target.size() <= str.size()){
  	return (0 == str.compare(0, target.size(), target));
  }
  return false;
}

/**@brief  append to a str only if need
 *
 * @param str The string to append to
 * @param app What to append to str
 * @return A reference to the str
 */
inline std::string & appendAsNeeded(std::string & str, const std::string & app){
	if(!endsWith(str,app)){
		str.append(app);

	}
	return str;
}

/**@brief Return str appended if required
 *
 * @param str The str to append to
 * @param app What to append to the str
 * @return str appended with app only if it is not already appended
 */
inline std::string appendAsNeededRet(std::string str, const std::string & app){
	if(!endsWith(str,app)){
		str.append(app);
	}
	return str;
}

/**@brief Determine if string has a whitespace character
 *
 * @param str The string to examine
 * @return Whether str has a whitepsace character
 */
inline bool strHasWhitesapce(const std::string & str){
	return std::any_of(std::begin(str), std::end(str),
			[](char c) {return std::isspace(c);});
}

} // namesapce bib
