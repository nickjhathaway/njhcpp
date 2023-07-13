#pragma once
/*
 * stringUtils.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <string>
#include <unordered_set>
#include <cppitertools/range.hpp> //range
#include <set>
#include <sstream> //std::stringstream
#include <iostream>
#include <iterator> //std::ostream_iterator
#include <cmath> //round()
#include <iomanip> //std::setFill(), std::setw()
#include <regex>
#include <utility>
#include "njhcpp/common.h" //estd::to_string
//#include "njhcpp/jsonUtils/jsonUtils.hpp" //included here so that most files will have json


namespace njh {



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

template <>
inline std::string conToStr(const std::vector<uint8_t>& con,
                     const std::string& delim) {
  if (con.empty()) {
    return "";
  }
  std::string ret = estd::to_string(con[0]);
  for(const auto pos : iter::range<uint64_t>(1, con.size())){
  	ret += delim;
  	ret += estd::to_string(con[pos]);
  }
  return ret;
}

template <>
inline std::string conToStr(const std::set<uint8_t>& con,
                     const std::string& delim) {
  if (con.empty()) {
    return "";
  }
  std::string ret = "";
  for(const auto val : con){
  	if("" != ret){
  		ret += delim;
  	}
  	ret += estd::to_string(val);
  }
  return ret;
}

template <>
inline std::string conToStr(const std::unordered_set<uint8_t>& con,
                     const std::string& delim) {
  if (con.empty()) {
    return "";
  }
  std::string ret = "";
  for(const auto val : con){
  	if("" != ret){
  		ret += delim;
  	}
  	ret += estd::to_string(val);
  }
  return ret;
}



/**@brief Turn a container into a delimited string with the last delimiter being something different
 *
 * @param container the container (std::vector, std::set, etc)
 * @param delim the delimiter for the majority of the string
 * @param lastDelim the last delimiter
 * @return a delimited string
 */
template<typename CON>
std::string conToStrEndSpecial(const CON & container,
		const std::string & delim, const std::string & lastDelim) {
	std::string ret = "";
	if (container.size() > 1) {
		uint32_t count = 0;
		for (const auto & element : container) {
			if (count + 1 == container.size()) {
				ret += lastDelim;
			} else if (count > 0) {
				ret += delim;
			}
			ret += estd::to_string(element);
			++count;
		}
	} else {
		ret = njh::conToStr(container, delim);
	}
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
  if(0 == highestNumber){
  		ret << num << std::endl;
  } else if (0 == num) {
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
	if("" == str){
		return std::vector<std::string>{};
	}
  std::vector<std::string> ret;
  if ("whitespace" == delim) {
    std::stringstream ss(str);
    while (!ss.eof()) {
      std::string tempStr;
      ss >> tempStr;
      ret.emplace_back(tempStr);
    }
  } else {
    if (std::string::npos == str.find(delim)  ) {
      ret.push_back(str);
    } else {
      std::size_t pos = str.find(delim, 0);
      std::size_t oldPos = -delim.size();
      while (std::string::npos != pos) {
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
  return (std::string::npos != str.find(subString));
}



/**@brief Converter for bool to true or false str
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
	if("" == toBeReplaced){
		//maybe throw? warn?
		return theString;
	}
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
    		[](int c) {return !std::isspace(c);}));
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
    		[](int c) {return !std::isspace(c);} ).base(), s.end());
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

/**@brief Get the longest string of all the elements in a container
 *
 * @param con The container
 * @param f Function to get an object out of the container
 * @return A string of the longest element if converted to strings
 */
template <typename CON, typename FUN>
std::string longestToString(const CON& con, FUN f){
    auto t = [&f](auto& e){ return estd::to_string(f(e)); };
    auto longest = std::max_element(con.begin(), con.end(), [&t](const auto& a, const auto& b){
            return t(a).size() < t(b).size();
        });
    return t(*longest);
}

/**@brief Get the length of the longest string representation in container by using a function to get elements
 *
 * @param con The container to query
 * @param f A function to get elements out of the container
 * @return The length of the longest string representation
 */
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


/**@brief Get the padding for two columns from a map
 *
 * @param theMap A map you're going to print
 * @param paddings The paddings to compare to and set
 * @param kFunc A value to get the keys in the map
 * @param vFunc A value to get the values in the map
 */
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

/**@brief Function to test if a string ends with another
 *
 * @param str The string to test
 * @param target The target string to see if str ends with this
 * @return Whether str ends with target
 */
inline bool endsWith(const std::string& str, const std::string& target) {
  // http://stackoverflow.com/a/874160
  if (str.size() >= target.size()) {
    return (0 == str.compare(str.size() - target.size(), target.size(), target));
  }
  return false;
}

/**@brief Function to test if a string starts with another
 *
 * @param str The string to test
 * @param target The target string to see if str starts with this
 * @return Whether str starts with target
 */
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


template<typename T>
void addAsStrToStr(std::string& str, const T& e) {
	str.append(estd::to_string(e));
}

template<typename T>
void addAsStrToStr(std::string& str, const std::vector<T>& items) {
	for (const auto& e : items) {
		str.append(estd::to_string(e));
	}
}

template<typename T>
void pasteAsStrAdd(std::string & str, const T& last) {
	addAsStrToStr(str, last);
}

template<typename N, typename ... T>
void pasteAsStrAdd(std::string& str, const N& next, const T&... rest) {
	addAsStrToStr(str, next);

	pasteAsStrAdd(str, rest...);
}

/**@brief Paste all objects as one string, will also unroll vectors
 *
 * @param items All things to paste together
 * @return A string of all the items pasted together
 */
template<typename ... T>
std::string pasteAsStr(const T&... items) {
	std::string ret = "";
	ret.reserve(sizeof...(items));
	pasteAsStrAdd(ret, items...);
	return ret;
}


/**@brief Remove all whitespace from string
 *
 * @param s The string to remove whitespace from
 * @return A universal reference to the result
 */
template<typename T>
T && removeAllWhitespace(T && s) {
	s.erase(
			std::remove_if(s.begin(), s.end(),
					[]( char ch ) {return std::isspace<char>( ch, std::locale::classic() );}),
			s.end());
	return std::forward<T>(s);
}

/**@brief backslash escape special characters \ /:*?"<>|;[]
 *
 * @param input the input with special characters to be replaced
 */
inline void escapeSpecialChars(std::string & input) {
	std::regex specialCharsPat {
		R"(([\+\^\$\ /:*?"<>|;\[\]]))"};
	std::string format{"\\$1"};
	input = std::regex_replace(input, specialCharsPat, format);
}

/**@brief backslash escape special characters \ /:*?"<>|;[]
 *
 * @param input the input with special characters to be replaced
 * @return the input string but with the special character replaced
 */
inline std::string escapeSpecialCharsRet(const std::string & input) {
	std::regex specialCharsPat {
		R"(([\+\^\$\ /:*?"<>|;\[\]]))"};
	std::string format{"\\$1"};
	return std::regex_replace(input, specialCharsPat, format);
}

/**@brief test if a string contains special characters \ /:*?"<>|;[]
 *
 * @param input string
 * @return true if input contains at least 1 special character
 */
inline bool containsSpecialChars(const std::string & input){
	std::regex specialCharsPat {
		R"(([\+\^\$\ /:*?"<>|;\[\]]))"};
	std::smatch match;
	return std::regex_search(input, match, specialCharsPat);
}

/**
 * @fn std::string replaceSpecialCharacters(const std::string&, const std::string&="-")
 * @brief replace special character within a string with another string
 *
 * @param input the input string
 * @param replacement the replacement string for the special characters
 * @return the input string with the special characters replaced
 */
inline std::string replaceSpecialCharacters(const std::string & input, const std::string & replacement = "-"){
	std::regex specialCharsPat {
		R"(([\+\^\$\ /:*?"<>|;\[\]]))"};
	return std::regex_replace(input, specialCharsPat, replacement);
}


/**@brief Returns true if string is all whitespace characters
 *
 * @param str the string to test
 * @return true if str is all whitespace chracters
 */
inline bool allWhiteSpaceStr(const std::string & str){
	return std::string::npos == str.find_first_not_of(" \f\n\r\t\v");
}



/**@brief Class for finding the position of certain regex patterns in strings
 *
 */
class PatPosFinder {
public:
	/**@brief A sub helper class to store the pattern found and the position at which it was found
	 *
	 */
	struct PatPosSize{
		/**@brief Construct with pattern and position
		 *
		 * @param pat the pattern itself found, not the regex pattern searched with e.g. AAAAA for regex A+ in TTAAAAATG
		 * @param pos the position at which the pattern can be found
		 */
		PatPosSize(std::string  pat, size_t pos): pat_(std::move(pat)), pos_(pos){

		}
		std::string pat_; /**< the pattern found*/
		size_t pos_; /**< the position at which it was found */

		/**@brief get the end position of the pattern, just pos_ + pat_.size()
		 *
		 * @return the end position, not inclusive
		 */
		[[nodiscard]] size_t end() const{
			return pos_ + pat_.size();
		}
	};

	/**@brief construct with a string that will be made into a regex pattern
	 *
	 * @param str the string that has a regex pattern
	 */
	PatPosFinder(const std::string & str): pat_(str){

	}

	/**@brief construct with a regex object
	 *
	 * @param pat the regex object
	 */
	PatPosFinder(std::regex  pat): pat_(std::move(pat)){

	}

	std::regex pat_; /**< the pattern to search with */

	/**@brief Get the positions of pat_ in input string, an empty vector means there was no matching pattern
	 *
	 * @param str the string to search in
	 * @return the results of the search, a vector of objects that contain the matching pattern and the position at which it can be found
	 */
	[[nodiscard]] std::vector<PatPosSize> getPatPositions(const std::string & str) const{
    std::sregex_iterator iter(str.begin(), str.end(), pat_);
    std::sregex_iterator end;
  	std::vector<PatPosSize> pats;
		while (iter != end) {
			pats.emplace_back((*iter)[0], iter->position());
			++iter;
		}
		return pats;
	}
};


/**@brief Tokenize a string with substrings matching pattern
 *
 * @param str The string to tokenize
 * @param regExp the expression to tokenize on
 * @return a vector of all sub strings matching the given regex pattern
 */
inline std::vector<std::string> tokStrOnMatchRegex(std::string & str, const std::regex & regExp){
  std::regex_token_iterator<std::string::iterator> rend;

  std::vector<std::string> toks;
  std::regex_token_iterator<std::string::iterator> it ( str.begin(), str.end(), regExp );
  while (it!=rend) {
  	toks.emplace_back(*it++);
  }
  return toks;
}

/**@brief Tokenize a string with substrings between matching pattern
 *
 * @param str The string to tokenize
 * @param regExp the expression to tokenize on
 * @return a vector of all substring inbetween the given regex pattern
 */
inline std::vector<std::string> tokStrOnSplitRegex(std::string & str, const std::regex & regExp){
  std::regex_token_iterator<std::string::iterator> rend;

  std::vector<std::string> toks;
  std::regex_token_iterator<std::string::iterator> it ( str.begin(), str.end(), regExp, -1);
  while (it!=rend) {
  	if(it->matched){
  		toks.emplace_back(*it++);
  	}else{
  		++it;
  	}
  }
  return toks;
}

/**@brief Natural sort a vector of names (e.g. putting Example9 before Example10)
 *
 * @param names The vector of names
 * @return a sorted vector of names
 */
inline std::vector<std::string> naturalSortNameRet(const std::vector<std::string> & names){
	struct NameWithNameSplit {
		explicit NameWithNameSplit(std::string  name):
						name_(std::move(name)){
			const std::regex regPat_{"([A-Za-z0-9\\.]+)" };
			const std::regex subPat_ {"([A-Za-z]*)([0-9\\.]*)"};

			nameToks_ = njh::tokStrOnMatchRegex(name_, regPat_);
			for(const auto & nameTok : nameToks_) {
				std::smatch nameMatch;
				if(!std::regex_match(nameTok.begin(), nameTok.end(), nameMatch, subPat_)) {
					//					std::stringstream ss;
					//					ss << __PRETTY_FUNCTION__ << ", error " << nameTok << "nameTok didn't match pattern"<< "\n";
					//					throw std::runtime_error{ss.str()};
					subNameToks_.emplace_back(nameTok, std::numeric_limits<double>::min() );
				} else {
					subNameToks_.emplace_back(nameMatch[1], ("" == nameMatch[2] ? std::numeric_limits<double>::min() :std::stod(nameMatch[2]) ) );
				}
			}
		}
		std::string name_;

		std::vector<std::string> nameToks_;
		std::vector<std::pair<std::string, double>> subNameToks_;
	};
	std::vector<NameWithNameSplit> namesWithSplit;
	namesWithSplit.reserve(names.size());
	for (const auto &name: names) {
		namesWithSplit.emplace_back(name);
	}
	njh::sort(namesWithSplit, []( const NameWithNameSplit & seq1, const NameWithNameSplit & seq2) {
		auto smallest = std::min(seq1.nameToks_.size(), seq2.nameToks_.size());
		for(uint32_t pos = 0; pos < smallest; ++pos) {
			if(seq1.subNameToks_[pos].first == seq2.subNameToks_[pos].first) {
				if(seq1.subNameToks_[pos].second != seq2.subNameToks_[pos].second) {
					return seq1.subNameToks_[pos].second < seq2.subNameToks_[pos].second;
				}
			} else {
				return seq1.subNameToks_[pos].first < seq2.subNameToks_[pos].first;
			}
		}
		return seq1.subNameToks_.size() < seq2.subNameToks_.size();
	});
	std::vector<std::string> ret;
	ret.reserve(namesWithSplit.size());
	for (const auto &nameSplit: namesWithSplit) {
		ret.emplace_back(nameSplit.name_);
	}
	return ret;
}

/**
 * @brief Sort the names in input vector and set the input vector to that sorted vector
 * @param names the input to be sorted
 * @return a reference to the input vector
 */
inline std::vector<std::string>& naturalSortNameSet(std::vector<std::string> & names){
	names = naturalSortNameRet(names);
	return names;
}

/**
 * @brief Generate a vector of the ansi "printable" characters 33-127
 * @return char vector of "printable" characters
 */
inline std::vector<char> genVecOfAnsiPrintable(){
	std::vector<char> ret(127-33);
	njh::iota(ret, static_cast<char>(33));
	return ret;
}

/**
 * @brief Generate a set of the ansi "printable" characters 33-127
 * @return char set of "printable" characters
 */
inline std::set<char> genSetOfAnsiPrintable(){
	auto chars = genVecOfAnsiPrintable();
	return std::set<char>{chars.begin(), chars.end()};
}

} // namesapce njh
