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

#include "bibcpp/common.h" //to_string, isArithmetic, isString
#include <boost/type_index.hpp>

namespace bib {
inline std::string prettierName(const std::string & prettyName){
	return bib::replaceString(bib::replaceString(bib::replaceString(prettyName,
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
public:
	bad_lexical_cast(const std::string & targetType, const std::string & sourceType) :
			targetType_(targetType), sourceType_(sourceType) {

	}

	virtual const char *what() const throw () {
		return std::string("bad lexical cast: "
				"source type value, " + sourceType_ +  ", could not be interpreted as target, " + targetType_).c_str();
	}

	virtual ~bad_lexical_cast() throw () {
	}

};


/**@brief Most likely won't be called, just so compilier doesn't complain about std::stod for non-string types
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
		throw bad_lexical_cast(
				prettierName(
						boost::typeindex::type_id_with_cvr<Target>().pretty_name()),
				prettierName(
						boost::typeindex::type_id_with_cvr<Source>().pretty_name()));
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
		throw bad_lexical_cast(
				prettierName(
						boost::typeindex::type_id_with_cvr<std::string>().pretty_name()),
				prettierName(
						boost::typeindex::type_id_with_cvr<Source>().pretty_name()));
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

}  // namespace bib
