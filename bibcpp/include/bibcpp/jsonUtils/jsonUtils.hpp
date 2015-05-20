#pragma once
/*
 * jsonUtils.hpp
 *
 *  Created on: Nov 23, 2014
 *      Author: nickhathaway
 */


#include <json/json.h>
#include <cppitertools/range.hpp>
#include <map>
#include <unordered_map>
#include <vector>
#include <deque>
#include <string>
#include <cstdint>

namespace bib{
namespace json{

template<typename CON>
inline Json::Value toJson(const CON & con);


template<typename FIRST, typename SECOND>
void toJsonHelper(const std::pair<FIRST,SECOND> & pair, Json::Value & ret){
	ret[estd::to_string(pair.first)] = toJson(pair.second);
}

template<typename T>
void toJsonHelper(const T & val, Json::Value & ret){
	ret.append(toJson(val));
}

template<typename CON>
Json::Value toJson(const CON & con){
	Json::Value ret;
	for(const auto & val : con){
		toJsonHelper(val,ret);
	}
	return ret;
}

template<>
inline Json::Value toJson(const std::string & val){
	Json::Value ret(val);
	return ret;
}

inline Json::Value toJson(const char * val){
	return toJson(std::string(val));
}

inline Json::Value toJson(char * val){
	return toJson(std::string(val));
}


template<>
inline Json::Value toJson(const uint8_t & val){
	Json::Value ret(val);
	return ret;
}
template<>
inline Json::Value toJson(const uint16_t & val){
	Json::Value ret(val);
	return ret;
}
template<>
inline Json::Value toJson(const uint32_t & val){
	Json::Value ret(val);
	return ret;
}
template<>
inline Json::Value toJson(const uint64_t & val){
	Json::UInt64 conversion = val;
	Json::Value ret(conversion);
	return ret;
}


#ifndef __linux__
template<>
inline Json::Value toJson(const size_t & val){
	Json::Value ret(static_cast<uint64_t>(val));
	return ret;
}
#endif
template<>
inline Json::Value toJson(const int8_t & val){
	Json::Value ret(val);
	return ret;
}
template<>
inline Json::Value toJson(const int16_t & val){
	Json::Value ret(val);
	return ret;
}
template<>
inline Json::Value toJson(const int32_t & val){
	Json::Value ret(val);
	return ret;
}

template<>
inline Json::Value toJson(const int64_t & val){
	Json::Int64 conversion = val;
	Json::Value ret(conversion);
	return ret;
}

template<>
inline Json::Value toJson(const double & val){
	Json::Value ret(val);
	return ret;
}

template<>
inline Json::Value toJson(const bool & val){
	Json::Value ret(val);
	return ret;
}




}//namespace json
}//namesapce bib
