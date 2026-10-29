#pragma once
/*
 * typeUtils.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */

#include "njhcpp/common.h"
#include <boost/filesystem/path.hpp>
#include <regex>

namespace njh {

namespace impl {
/**@brief Getting the type out of the __PRETTY_FUNCTION__ of typeStr<T>()
 *
 * @param pf the __PRETTY_FUNCTION__ of typeStr<T>()
 * @return The type
 */
inline std::string typeStrFromPrettyFunction(std::string pf){
	static const std::regex namespacePat{"std::__1::"};
	static const std::regex namespace2Pat{"std::__cxx11::"};
	static const std::regex stringPat{"std::basic_string<char>"};
	static const std::regex lessPat{", std::less<.*?>"};
	static const std::regex allocPat{", std::allocator<.*?>"};
	static const std::regex spaceArrowPat{" >"};
	pf = std::regex_replace(pf, namespacePat, "std::");
	pf = std::regex_replace(pf, namespace2Pat, "std::");
	pf = std::regex_replace(pf, stringPat, "std::string");
	pf = std::regex_replace(pf, lessPat, "");
	pf = std::regex_replace(pf, allocPat, "");
	pf = std::regex_replace(pf, spaceArrowPat, ">");
	auto tEqualPos = pf.rfind("T = ");
	auto semiColPos = pf.find(";", tEqualPos);
	auto closeBracPos = pf.rfind("]");
	if(tEqualPos != std::string::npos &&
			closeBracPos != std::string::npos &&
			closeBracPos > tEqualPos){
		//this is for the difference in gcc and clang output of __PRETTY_FUNCTION__
		if(semiColPos != std::string::npos){
			return pf.substr(tEqualPos + 4, semiColPos - 4 - tEqualPos);
		}else{
			return pf.substr(tEqualPos + 4, closeBracPos - 4 - tEqualPos);
		}
	}else{
		return "indeterminate";
	}
}
}  // namespace impl

/**@brief Getting type by taking advantage of info available in __PRETTY_FUNCTION__
 *
 * @return The type
 */
template<typename T>
std::string typeStr(){
	static const std::string ret = impl::typeStrFromPrettyFunction(__PRETTY_FUNCTION__);
	return ret;
}


/**@brief Getting type of a given object by taking advantage of info available in __PRETTY_FUNCTION__
 *
 * @param val The object to get the type of
 * @return The type
 */
template<typename T>
std::string typeStr(const T & val){
	return typeStr<T>();
}


/**@brief A struct to get a string of what the value of a object is
 *
 */
struct TypeName
{

	template<typename U>
	static std::string get(const U & val){
		return typeStr(val);
	}

	template<typename U>
	static std::string get(){
		return typeStr<U>();
	}

};


// add more specializations for the build-in types
template<>
inline std::string TypeName::get<int16_t>() {return "int16_t";}
template<>
inline std::string TypeName::get<int32_t>() {return "int32_t";}
template<>
inline std::string TypeName::get<int64_t>() {return "int64_t";}
template<>
inline std::string TypeName::get<uint16_t>() {return "uint16_t";}
template<>
inline std::string TypeName::get<uint32_t>() {return "uint32_t";}
#if defined( __APPLE__ ) || defined( __APPLE_CC__ ) || defined( macintosh ) || defined( __MACH__ )
template<>
inline std::string TypeName::get<uint64_t>() {return "uint64_t";}
#endif
template<>
inline std::string TypeName::get<size_t>() {return "size_t";}
template<>
inline std::string TypeName::get<bool>() {return "bool";}
template<>
inline std::string TypeName::get<double>() {return "double";}
template<>
inline std::string TypeName::get<long double>() {return "long double";}
template<>
inline std::string TypeName::get<float>() {return "float";}
template<>
inline std::string TypeName::get<std::string>() {return "std::string";}


/**@brief A function to return the type of the obj input in string fomat
 *
 * @param obj The obj to get the type info from
 * @return a string of the object type
 */
template<typename T>
inline std::string getTypeName(const T & obj){
	return TypeName::get<T>();
}

}  // namespace njh



