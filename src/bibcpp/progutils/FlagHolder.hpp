#pragma once
/*
 * FlagHolder.hpp
 *
 *  Created on: Feb 3, 2016
 *      Author: nick
 */


#include "bibcpp/progutils/Flag.hpp"
#include <map>

namespace bib {
namespace progutils {

/**@brief A class for holding all the parameters
 *
 */
class flagHolder {

public:
	/**@brief Default constructor for an empty holder
	 *
	 */
	flagHolder() {
	}

	/**@brief A vector holding all the parameters
	 *
	 */
	std::map<std::string, Flag> flags_;
	/**Add a parameter that was already constructed
	 *
	 * @param newParameter A parameter object to add to the holder
	 */
	void addFlag(const Flag& newFlag) {
		for (const auto & flagStr : newFlag.flags_) {
			for (const auto & flagObj : flags_) {
				if (in(flagStr, flagObj.second.flags_)) {
					std::stringstream ss;
					ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding new flag: "
							<< flagStr << " which already exists" << std::endl;
					ss << flagObj.second.toJson() << std::endl;
					throw std::runtime_error { ss.str() };
				}
			}
		}
		flags_.emplace(conToStr(newFlag.flags_, ","), newFlag);
	}

	/**@brief Output the information for all stored parameters
	 *
	 * @param out An std::ostream object to print the info to
	 *
	 */
	void outputParsFile(std::ostream& out) {
		if (!flags_.empty()) {
			out << Flag::fullInfoHeader("\t") << std::endl;
			for (const auto & f : flags_) {
				out << f.second.fullInfo("\t") << std::endl;
			}
		}
	}

	/**@brief Get flags
	 *
	 * @return A vector of flags
	 */
	std::vector<std::string> getFlags()const{
		std::vector<std::string> ret;
		for(const auto & f : flags_){
			addOtherVec(ret, f.second.getFlagsStrs());
		}
		return ret;
	}

	/**@brief Get flags with auto prepending dashes based on flag length
	 *
	 * @return A vector of flags
	 */
	std::vector<std::string> getFlagsAutoDash()const{
		std::vector<std::string> ret;
		for(const auto & f : flags_){
			addOtherVec(ret, f.second.getFlagsStrsAutoDash());
		}
		return ret;
	}
};


}  // namespace progutils
}  // namespace bib

