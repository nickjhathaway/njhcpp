#pragma once
/*
 * typeUtils.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */

#include "njhcpp/common.h"
#include <boost/filesystem/path.hpp>

namespace njh {

/**@brief Getting type by taking advantage of info available in __PRETTY_FUNCTION__
 *
 * @return The type
 */
template<typename T>
std::string typeStr(){
	std::string pf(__PRETTY_FUNCTION__);
	std::cout << pf << std::endl;
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



