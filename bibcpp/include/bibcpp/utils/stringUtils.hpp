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

/**@b Check a string for several regex patterns
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

/**@b Check a string for several regex patterns and return false if the string contains any of them
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



/**@b Pad a number so that it sorts well with the rest of the numbers
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

/**@b return a vector of strings by spiting a string on a delimiter
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

/**@b Test to see if string contains all digits
 *
 * @param str The string to test
 * @return Return true if all chars in str is a digit
 */
inline bool strAllDigits(const std::string& str) {
  for (const auto& c : str) {
    if (!isdigit(c)) {
      return false;
    }
  }
  return true;
}

/**@b Just a convience to check to see if a str contains a substring
 *
 * @param str the string to check
 * @param subString the substring to check for
 * @return True if str contains subString
 */
inline bool containsSubString(const std::string& str,
                              const std::string& subString) {
  return (str.find(subString) != std::string::npos);
}

/**@b Return a string with the text centered for the max width of the line
 * @todo Check that text size is shorter than maxWidth
 * @param text The text to center
 * @param maxWidth The max width of the line
 * @return A new string with the text centered for the maxWidth
 */
inline std::string centerText(const std::string& text, uint32_t maxWidth) {
  uint32_t halfWay = round(maxWidth / 2.0);
  uint32_t halfText = round(text.size() / 2.0);
  return std::string(halfWay - halfText, ' ') + text;
}

/**@b Convertor for bool to true or false str
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


/**@b Convert a string to all upper case letters
 *
 * @param str String to convert
 */
inline void strToUpper(std::string& str) {
  for (auto& c : str) {
    c = toupper(c);
  }
}
/**@b Convert a string to all lower case letters
 *
 * @param str String to convert
 */
inline void strToLower(std::string& str) {
  for (auto& c : str) {
    c = tolower(c);
  }
}

/**@b Convert a string to all upper case letters, leave original alone
 *
 * @param str String to convert
 * @return str but with all upper case letters
 */
inline std::string strToUpperRet(std::string str) {
  strToUpper(str);
  return str;
}

/**@b  Convert a string to all lower case letters, leave original alone
 *
 * @param str String to convert
 * @return str but with all lower case letters
 */
inline std::string strToLowerRet(std::string str) {
  strToLower(str);
  return str;
}

/**@b Convert a vector of strings to all lower case letters
 *
 * @param vec Vector to convert
 */
inline void strVecToLower(std::vector<std::string>& vec) {
  for (auto& v : vec) {
    strToLower(v);
  }
}

/**@b Replace a substring in a string with a new substring
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

/**@b convert int to a string hex string
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

/**@b convert a hexstring to a int
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




} // namesapce bib
