#pragma once
/*
 * flag.hpp
 *
 *  Created on: Sep 22, 2014
 *      Author: nickhathaway
 */

#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/stdAddition/misc.hpp"

namespace bib {
namespace progutils {
template<class T>
struct TypeName
{
static std::string get() { return T::typeName(); }
};
// add more specializations for the build-in types
template<>
inline std::string TypeName<int16_t>::get() {return "int16_t";}
template<>
inline std::string TypeName<int32_t>::get() {return "int32_t";}
template<>
inline std::string TypeName<int64_t>::get() {return "int64_t";}
template<>
inline std::string TypeName<uint16_t>::get() {return "uint16_t";}
template<>
inline std::string TypeName<uint32_t>::get() {return "uint32_t";}
template<>
inline std::string TypeName<uint64_t>::get() {return "uint64_t";}
#ifndef __linux__
template<>
inline std::string TypeName<size_t>::get() {return "size_t";}
#endif
template<>
inline std::string TypeName<bool>::get() {return "bool";}
template<>
inline std::string TypeName<double>::get() {return "double";}
template<>
inline std::string TypeName<long double>::get() {return "long double";}
template<>
inline std::string TypeName<float>::get() {return "float";}
template<>
inline std::string TypeName<std::string>::get() {return "string";}

template<typename T>
inline std::string getTypeName(const T & obj){
	return TypeName<T>::get();
}
class flag {
public:
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

	std::vector<std::string> flags_;
	std::string shortDescription_;
	bool required_;
	bool set_;
	std::string setValue_;
	std::string defaultValue_;
	std::string type_;

	template<typename T>
	void setValue(const T & option){
		if(getTypeName(option) != type_){
			std::cerr << "Bad type set for flag " << conToStr(flags_,",") << "\n"
					<< "is of type " << type_ << " and trying to set with type: " << getTypeName(option)
			<< "\n";
			exit(1);
		} else {
			setValue_ = estd::to_string(option);
			set_ = true;
		}
	}


	std::string fullInfo(const std::string delim = "\t", bool header = false) const {
    if (header) {
      return "ShortDescription\tvalue\tdefaultValue";
    }
    return shortDescription_
    		+ delim + setValue_
    		+ delim + defaultValue_;
	}
	std::string helpInfo()const {
		return shortDescription_ + "; default=" + defaultValue_ + "; (" + type_ + ")";
	}

};

/**A class for holding all the parameters
 *
 */
class flagHolder {

 public:
  /** Default constructor for an empty holder
   *
   */
	flagHolder() {}

  /**A vector holding all the parameters
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

  /** Output the information for all stored parameters
   *
   * @param out An std::ostream object to print the info to
   *
   */
  void outputParsFile(std::ostream& out) {
  	if(!flags_.empty()){
      flags_.begin()->second.fullInfo("\t", true);
      for (const auto & f : flags_) {
        out << f.second.fullInfo("\t") << std::endl;
      }
  	}
  }
};


}  // namespace progutils
}  // namespace bib
