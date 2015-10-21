#pragma once
/*
 * flag.hpp
 *
 *  Created on: Sep 22, 2014
 *      Author: nickhathaway
 */

#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/stdAddition/misc.hpp"
#include "bibcpp/bashUtils.h"
#include <map>

namespace bib {
/**@brief Getting type by taking advantage of info available in __PRETTY_FUNCTION__
 *
 * @return The type
 */
template<typename T>
std::string typeStr(){
	std::string pf(__PRETTY_FUNCTION__);
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

namespace progutils {
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


/**@brief Class hold commandline flag information
 *
 */
class flag {
public:
	/**@brief construct with templated option, so any value can be given that can be converted to a string
	 *
	 * @param opt The option value associated with the flag
	 * @param flags The command line flags that can set the option value
	 * @param shortDescription A short description of the flag/value
	 * @param required Whether the flag is required or not
	 */
	template<typename T>
	flag(const T & opt,
			const std::string & flags,
			const std::string & shortDescription,
			bool required):
		flags_(tokenizeString(flags,",")),
		shortDescription_(shortDescription),
		required_(required),
		set_(false),
		setValue_(estd::to_string(opt)),
		defaultValue_(estd::to_string(opt)),
		type_(getTypeName(opt))
		{
	}

	std::vector<std::string> flags_; /**<The flags associated with this option */
	std::string shortDescription_; /**<A short description for this option */
	bool required_; /**<Whether option is required or not */
	bool set_; /**<Whether the option was set by commandline or default value*/
	std::string setValue_; /**<If set by commandline what it was set to */
	std::string defaultValue_; /**<The default value associated with this otpion */
	std::string type_; /**<The type that the option, ex. int32_t,bool,etc */

	/**@brief Function to set the option with a new value from commandline
	 *
	 * @param option The new value for this option
	 */
	template<typename T>
	void setValue(const T & option){
		if(getTypeName(option) != type_){
			std::stringstream ss;
			ss << "Bad type set for flag " << conToStr(flags_,",") << "\n"
					<< "is of type " << type_ << " and trying to set with type: " << getTypeName(option)
			<< "\n";
			throw std::runtime_error{ss.str()};
		} else {
			setValue_ = estd::to_string(option);
			set_ = true;
		}
	}

	/**@brief Print a info on option for a file parameters used file
	 *
	 * @param delim The delimiter to use
	 * @param header Whether to just print the header
	 * @return A string with the info
	 */
	std::string fullInfo(const std::string delim = "\t", bool header = false) const {
    if (header) {
      return "Flags\tShortDescription\tvalue\tdefaultValue";
    }
    return conToStr(flags_, ",")
    		+ delim + shortDescription_
    		+ delim + setValue_
    		+ delim + defaultValue_;
	}
	/**@brief For printing info to the terminal with color output
	 *
	 * @return a string with the info
	 */
	std::string helpInfo()const {
		std::string out =  shortDescription_ + "; ";
		if(required_){
			out +=  bashCT::bold + bashCT::red + "required"
					+ bashCT::reset +"; ";
		}
		out+=bashCT::bold + "default=" + bashCT::blue + defaultValue_
				+ bashCT::reset +"; "
						"(" + bashCT::addColor(202) + bashCT::bold + type_
						+ bashCT::reset +")";

		return out;
	}

};

/**@brief A class for holding all the parameters
 *
 */
class flagHolder {

 public:
  /**@brief Default constructor for an empty holder
   *
   */
	flagHolder() {}

  /**@brief A vector holding all the parameters
   *
   */
  std::map<std::string, flag> flags_;
  /**Add a parameter that was already constructed
   *
   * @param newParameter A parameter object to add to the holder
   */
  void addFlag(const flag& newFlag) {
  	flags_.emplace(conToStr(newFlag.flags_,","), newFlag);
  }

  /**@brief Output the information for all stored parameters
   *
   * @param out An std::ostream object to print the info to
   *
   */
  void outputParsFile(std::ostream& out) {
  	if(!flags_.empty()){
      out << flags_.begin()->second.fullInfo("\t", true) << std::endl;
      for (const auto & f : flags_) {
        out << f.second.fullInfo("\t") << std::endl;
      }
  	}
  }
};


}  // namespace progutils
}  // namespace bib
