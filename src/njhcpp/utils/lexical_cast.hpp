#pragma once
/*
 * lexical_cast.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <typeinfo> //std::bad_cast, typeid, std::type_info
#include <string>
#include <algorithm> //std::transform

#include "njhcpp/common.h" //to_string, isArithmetic, isString
#include "njhcpp/utils/typeUtils.hpp"
#include "njhcpp/utils/stringUtils.hpp"


namespace njh {
inline std::string prettierName(const std::string & prettyName){
	return njh::replaceString(njh::replaceString(njh::replaceString(prettyName,
			"std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >", "std::string"),
			"std::__1", "std"),
			" >", ">");
}




/*Lexical cast adapted from zi_lib lexical cast implementation but adapted for
 * conversion of strings in scientific to arithmetic type -nick
 *
 */

/**@brief Exception for bad lexical cast
 *
 */


class bad_lexical_cast: public std::bad_cast {
private:
	std::string targetType_;
	std::string sourceType_;
	std::string message_;
public:
	bad_lexical_cast(const std::string & targetType, const std::string & sourceType) :
			targetType_(targetType), sourceType_(sourceType), message_("bad lexical cast: source type value, " + sourceType_ +  ", could not be interpreted as target, " + targetType_){

	}

	virtual const char *what() const throw () {
		return message_.c_str();
	}

	virtual ~bad_lexical_cast() throw () {
	}

};


/**@brief Most likely won't be called, just so compiler doesn't complain about std::stod for non-string types
 *
 */
template< class Source>
inline std::string toSreamHelper(const Source & source){
	return estd::to_string(source);
}

/**@brief For lexical cast to convert scientific notation properly
 *
 */
inline std::string toSreamHelper(const std::string & source){
  std::stringstream ss;
  ss << std::setprecision(20) << std::stod(source);
	return ss.str();
}

/**@brief Caster for to anything not a string, with a check for if it is a cast from
 * a string to a arithmetic type to help with scientific notation
 *
 *
 */
template<class Target, class Source>
struct lexical_caster {
	static inline Target cast_it(const Source& source) {
		Target ret;
		std::stringstream ss;
		if (estd::isArithmetic<Target>() && estd::isString<Source>()) {
			ss << toSreamHelper(source);
		} else {
			ss << source;
		}
		if (ss && ss >> ret) {
			if (ss.eof()) {
				return ret;
			}
		}
		throw bad_lexical_cast(typeStr<Target>(), typeStr<Source>());
	}
};

/**@brief Caster for anything to string, just take the value of the stream
 *
 */
template<class Source>
struct lexical_caster<std::string, Source> {
	static inline std::string cast_it(const Source& source) {
		std::ostringstream oss;
		if (oss << source) {
			return oss.str();
		}
		throw bad_lexical_cast(typeStr<std::string>(), typeStr<Source>());
	}
};




/**@brief The actual function to call to cast
 *
 * @param source What's to be cast
 * @return the cast of source to type Target
 */
template< class Target, class Source >
Target lexical_cast( const Source& source )
{
	return lexical_caster< Target, Source >::cast_it( source );
}


/**@brief Lexical cast to do entire container
 *
 * @param con The container of elements to be cast
 * @return A new container with the cast values
 */
template<typename SourceCon, typename OutCon>
OutCon lexical_cast_con(const SourceCon & con){
	OutCon ret(con.size());
	std::transform(con.begin(), con.end(), ret.begin(),
			[](const typename SourceCon::value_type & e){
		return lexical_cast<typename OutCon::value_type,typename SourceCon::value_type>(e);});
	return ret;
}

namespace StrToNumConverter {

/**@brief Function for converting a string to a number, which is just njh::lexical_cast by default and then several specific int conversions are defined for faster converting
 *
 * @param str the string to convert
 * @return the string convert to a number
 */
	template<typename T>
	T stoToNum(const std::string & str){
		return njh::lexical_cast<T>(str);
	}

	template<unsigned short>
	unsigned short stoToNum(const std::string & str){
		return estd::stous(str);
	}

	template<unsigned>
	unsigned stoToNum(const std::string & str){
		return estd::stou(str);
	}

	template<unsigned long>
	unsigned long stoToNum(const std::string & str){
		return std::stoul(str);
	}

	template<unsigned long long>
	unsigned long stoToNum(const std::string & str){
		return std::stoull(str);
	}

	template<short>
	short stoToNum(const std::string & str){
		return estd::stos(str);
	}

	template<int>
	int stoToNum(const std::string & str){
		return std::stoi(str);
	}

	template<long int>
	long int stoToNum(const std::string & str){
		return std::stol(str);
	}

	template<long long int>
	long long int stoToNum(const std::string & str){
		return std::stoll(str);
	}

}  // namespace StrToNumConverter


/**@brief convert a delimted string into a std::set
 *
 * @param str the string to convert
 * @param delim the delimiter
 * @return a std::set of the input delimited string
 */
template<typename T>
std::set<T> strToSet(const std::string & str, const std::string & delim){
	if("" == str){
		return std::set<T>{};
	}
	auto toks = tokenizeString(str, delim);
	std::set<T> ret;
	for(const auto & tok : toks){
		ret.emplace(lexical_cast<T>(tok));
	}
	return ret;
}


}  // namespace njh
