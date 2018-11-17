#pragma once
/*
 * flag.hpp
 *
 *  Created on: Sep 22, 2014
 *      Author: nickhathaway
 */

#include "njhcpp/utils.h"
#include "njhcpp/bashUtils.h"
#include "njhcpp/common/misc.hpp"

namespace njh {
namespace progutils {


class OptionToStr {
public:

	template<typename T>
	struct is_numeric_primitive : public std::integral_constant<bool,
	   std::is_same<int, typename std::decay<T>::type>::value
	|| std::is_same<short, typename std::decay<T>::type>::value
	|| std::is_same<int, typename std::decay<T>::type>::value
	|| std::is_same<long, typename std::decay<T>::type>::value
	|| std::is_same<long long, typename std::decay<T>::type>::value
	|| std::is_same<unsigned short, typename std::decay<T>::type>::value
	|| std::is_same<unsigned int, typename std::decay<T>::type>::value
	|| std::is_same<unsigned long, typename std::decay<T>::type>::value
	|| std::is_same<double, typename std::decay<T>::type>::value
	|| std::is_same<long double, typename std::decay<T>::type>::value
	|| std::is_same<float, typename std::decay<T>::type>::value
	> {};

	template<typename T>
	static constexpr bool isNumericPrimitive(){
		return is_numeric_primitive<T>::value;
	}

	template<typename T>
	static std::string optionToStr(const T & option){
		return optionToStr_impl(option);
	}

	template<typename T>
	static std::string optionToStr_impl(const T & option){
		if(isNumericPrimitive<T>() && std::numeric_limits<T>::max() == option){
			return "None";
		}else{
			return estd::to_string(option);
		}
	}

	template<typename T>
	static std::string optionToStr_impl(const std::vector<T> & option){
		return option.empty() ? "None" : conToStr(option, ",");
	}

	template<typename T>
	static std::string optionToStr_impl(const std::set<T> & option){
		return option.empty() ? "None" : conToStr(option, ",");
	}

};


/**@brief Class hold commandline flag information
 *
 */
class Flag {
public:
	/**@brief construct with templated option, so any value can be given that can be converted to a string
	 *
	 * @param opt The option value associated with the flag
	 * @param flags The command line flags that can set the option value
	 * @param shortDescription A short description of the flag/value
	 * @param required Whether the flag is required or not
	 */
	template<typename T>
	Flag(const T & opt, const std::string & flags,
			const std::string & shortDescription, bool required) :
			 shortDescription_(shortDescription), required_(
					required), set_(false), setValue_(OptionToStr::optionToStr(opt)), defaultValue_(
							OptionToStr::optionToStr(opt)), type_(getTypeName(opt)) {
		initialize(flags);
//		if(isNumericPrimitive<T>() && std::numeric_limits<T>::max() == opt){
//			defaultValue_ = "None";
//		}
	}
	/**@brief construct with templated option, so any value can be given that can be converted to a string
	 *
	 * @param opt The option value associated with the flag
	 * @param flags The command line flags that can set the option value
	 * @param shortDescription A short description of the flag/value
	 * @param required Whether the flag is required or not
	 * @param flagGrouping a grouping for the flag
	 */
	template<typename T>
	Flag(const T & opt, const std::string & flags,
			const std::string & shortDescription, bool required,
			const std::string flagGrouping) :
			shortDescription_(shortDescription), required_(required), set_(false), setValue_(
					OptionToStr::optionToStr(opt)), defaultValue_(OptionToStr::optionToStr(opt)), type_(
					getTypeName(opt)), flagGrouping_(flagGrouping) {
		initialize(flags);
//		if(isNumericPrimitive<T>() && std::numeric_limits<T>::max() == opt){
//			defaultValue_ = "None";
//		}
	}

	/**@brief initialize the flag objects with the input flags
	 *
	 * @param flags a string of comma separated flags
	 */
	void initialize(const std::string & flags){
		auto flagToks = tokenizeString(flags, ",");
		for(const auto & flagTok : flagToks){
			if(flagTok.empty()){
				std::stringstream ss;
				ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding a empty flag" << std::endl;
				ss << "flags:" << flags << std::endl;
				throw std::runtime_error{ss.str()};
			}
			if('-' != flagTok.front()){
				std::stringstream ss;
				ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding a flag that doens't start with a dash" << std::endl;
				ss << "flags:" << flags << std::endl;
				throw std::runtime_error{ss.str()};
			}
			auto pos = flagTok.find_first_not_of("-");

			if(std::string::npos == pos){
				std::stringstream ss;
				ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding a flag made up of only dashes" << std::endl;
				ss << "flags:" << flags << std::endl;
				throw std::runtime_error{ss.str()};
			}
			if(has(flags_, flagTok, [](const std::string & conVal, const std::string & inVal){ return strToLowerRet(lstripRet(conVal, '-')) == strToLowerRet(lstripRet(inVal, '-'));})){
				std::stringstream ss;
				ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding a flag that was already added, " << flagTok << std::endl;
				ss << "flags: " << flags << std::endl;
				throw std::runtime_error{ss.str()};
			}
			flags_.emplace_back(flagTok);
		}
	}

	std::vector<std::string> flags_; /**< The flags associated with this option */
	std::string shortDescription_; /**< A short description for this option */
	bool required_; /**< Whether option is required or not */
	bool set_; /**< Whether the option was set by commandline or default value*/
	std::string setValue_; /**< If set by commandline what it was set to */
	std::string defaultValue_; /**< The default value associated with this option */
	std::string type_; /**< The type that the option, ex. int32_t, bool, etc */
	std::string flagGrouping_ = "Misc"; /**< a grouping for the flag for when printing help messages*/

	/**@brief convert to json representation
	 *
	 * @return Json::Value object
	 */
	Json::Value toJson() const {
		Json::Value ret;
		ret["class"] = getTypeName(*this);
		ret["flags_"] = njh::json::toJson(flags_);
		ret["shortDescription_"] = njh::json::toJson(shortDescription_);
		ret["required_"] = njh::json::toJson(required_);
		ret["set_"] = njh::json::toJson(set_);
		ret["setValue_"] = njh::json::toJson(setValue_);
		ret["defaultValue_"] = njh::json::toJson(defaultValue_);
		ret["type_"] = njh::json::toJson(type_);
		ret["flagGrouping_"] = njh::json::toJson(flagGrouping_);

		return ret;
	}

	std::vector<std::string> flagsNoDash() const {
		std::vector<std::string> ret;
		for (const auto & flagStr : flags_) {
			auto pos = flagStr.find_first_not_of("-");
			if(std::string::npos == pos){
				std::stringstream ss;
				ss << "Error in : " << __PRETTY_FUNCTION__ << ", have a flag with only dashes" << std::endl;
				ss << toJson() << std::endl;
				throw std::runtime_error{ss.str()};
			}
			ret.emplace_back(flagStr.substr(pos));
		}
		sort(ret,
				[](const std::string & str1, const std::string & str2) {return str1.size() > str2.size();});
		return ret;
	}

	/**@brief Function to set the option with a new value from commandline
	 *
	 * @param option The new value for this option
	 */
	template<typename T>
	void setValue(const T & option) {
		if (getTypeName(option) != type_) {
			std::stringstream ss;
			ss << "Bad type set for flag " << conToStr(flags_, ",") << "\n"
					<< "is of type " << type_ << " and trying to set with type: "
					<< getTypeName(option) << "\n";
			throw std::runtime_error { ss.str() };
		} else {
			//setValue_ = estd::to_string(option);
			setValue_ = OptionToStr::optionToStr(option);
			set_ = true;
		}
	}
	static std::string fullInfoHeader(const std::string& delim) {
		return conToStr(std::vector<std::string> { "Flags", "ShortDescription",
				"value", "defaultValue" }, delim);
	}
	/**@brief Print a info on option for a file parameters used file
	 *
	 * @param delim The delimiter to use
	 * @param header Whether to just print the header
	 * @return A string with the info
	 */
	std::string fullInfoAutoDash(const std::string& delim) const {
		return getFlagsStrAutoDash() + delim + shortDescription_ + delim + setValue_
				+ delim + defaultValue_;
	}

	/**@brief Print a info on option for a file parameters used file
	 *
	 * @param delim The delimiter to use
	 * @param header Whether to just print the header
	 * @return A string with the info
	 */
	std::string fullInfo(const std::string& delim) const {
		return getFlagsStrAutoDash() + delim + shortDescription_ + delim + setValue_
				+ delim + defaultValue_;
	}
	/**@brief For printing info to the terminal with color output
	 *
	 * @return a string with the info
	 */
	std::string helpInfo() const {
		std::string out = shortDescription_ + "; ";
		if (required_) {
			out += bashCT::bold + bashCT::red + "required" + bashCT::reset + "; ";
		}
		std::string dValue = defaultValue_;

		out += bashCT::bold + "default=" + bashCT::blue + dValue
				+ bashCT::reset + "; "
						"(" + bashCT::addColor(202) + bashCT::bold + type_ + bashCT::reset
				+ ")";
		return out;
	}

//	template<typename T>
//	struct is_numeric_primitive : public std::integral_constant<bool,
//	   std::is_same<int, typename std::decay<T>::type>::value
//	|| std::is_same<short, typename std::decay<T>::type>::value
//	|| std::is_same<int, typename std::decay<T>::type>::value
//	|| std::is_same<long, typename std::decay<T>::type>::value
//	|| std::is_same<long long, typename std::decay<T>::type>::value
//	|| std::is_same<unsigned short, typename std::decay<T>::type>::value
//	|| std::is_same<unsigned int, typename std::decay<T>::type>::value
//	|| std::is_same<unsigned long, typename std::decay<T>::type>::value
//	|| std::is_same<double, typename std::decay<T>::type>::value
//	|| std::is_same<long double, typename std::decay<T>::type>::value
//	|| std::is_same<float, typename std::decay<T>::type>::value
//	> {};
//
//	template<typename T>
//	static constexpr bool isNumericPrimitive(){
//		return is_numeric_primitive<T>::value;
//	}

	/**@brief Get a comma delimited string of the flags while automating the number of dashes to have bashed on length
	 *
	 * @return A string of the flags
	 */
	std::string getFlagsStrAutoDash() const {
		auto flagsWithNoDash = flagsNoDash();
		std::string ret;
		for(const auto & flagStr : flagsWithNoDash){
			if(!ret.empty()){
				ret+= ",";
			}
			if(flagStr.size() == 1){
				ret += "-" + flagStr;
			}else{
				ret += "--" + flagStr;
			}
		}
		return ret;
	}

	/**@brief Get a vector of the flags while automating the number of dashes to have bashed on length
	 *
	 * @return A vector of the flags
	 */
	std::vector<std::string> getFlagsStrsAutoDash() const {
		std::vector<std::string> ret;
		auto flagsWithNoDash = flagsNoDash();
		for(const auto & flagStr : flagsWithNoDash){
			if(flagStr.size() == 1){
				ret.emplace_back("-" + flagStr);
			}else{
				ret.emplace_back("--" + flagStr);
			}
		}
		return ret;
	}

	/**@brief Get a comma delimited string of the flags
	 *
	 * @return A string of the flags
	 */
	std::string getFlagsStr() const {
		std::string ret;
		for(const auto & flagStr : flags_){
			if(!ret.empty()){
				ret+= ",";
			}
			ret += flagStr;
		}
		return ret;
	}

	/**@brief Get the flags
	 *
	 * @return a vector of the flags
	 */
	std::vector<std::string> getFlagsStrs() const {
		return flags_;
	}

	/**@brief Get a UID by concatenating the flags with auto dashing for flag length and sorting and using lower case
	 *
	 * @return a UID
	 */
	std::string getUIDAutoDash() const {
		auto flagsAutoDash = getFlagsStrsAutoDash();
		for(auto & f : flagsAutoDash){
			strToLower(f);
		}
		sort(flagsAutoDash);
		return conToStr(flagsAutoDash, ",");
	}

	/**@brief Auto add dashes to flag by length of str
	 *
	 * @param str the flag to add dashes to
	 * @return the flag with dashes
	 */
	static std::string autoDashFlag(std::string str){
		lstrip(str, '-');
		if(str.size() > 1){
			str = "--" + str;
		}else{
			str = "-" + str;
		}
		return str;
	}

};

}  // namespace progutils
}  // namespace njh
