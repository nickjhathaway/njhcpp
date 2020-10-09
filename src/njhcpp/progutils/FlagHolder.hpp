#pragma once
/*
 * FlagHolder.hpp
 *
 *  Created on: Feb 3, 2016
 *      Author: nick
 */

#include "njhcpp/progutils/Flag.hpp"
#include <map>

namespace njh {
namespace progutils {

/**@brief A class for holding all the parameters
 *
 */
class FlagHolder {

public:
	/**@brief Default constructor for an empty holder
	 *
	 */
	FlagHolder() {
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
		hasFlagDashCaseInsenThrow(newFlag);
		flags_.emplace(newFlag.getUIDAutoDash(), newFlag);
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
	std::vector<std::string> getFlags() const {
		std::vector<std::string> ret;
		for (const auto & f : flags_) {
			addOtherVec(ret, f.second.getFlagsStrs());
		}
		return ret;
	}

	/**@brief Get flags with no dashes
	 *
	 * @return A vector of flags with no dashes
	 */
	std::vector<std::string> getFlagsNoDash() const {
		std::vector<std::string> ret;
		for (const auto & f : flags_) {
			addOtherVec(ret, f.second.flagsNoDash());
		}
		return ret;
	}

	/**@brief Get flags with auto prepending dashes based on flag length
	 *
	 * @return A vector of flags
	 */
	std::vector<std::string> getFlagsAutoDash() const {
		std::vector<std::string> ret;
		for (const auto & f : flags_) {
			addOtherVec(ret, f.second.getFlagsStrsAutoDash());
		}
		return ret;
	}

	/**@brief Check to see if any of the other flags already have any of the flags for this flag option
	 *
	 * @param newFlag The flag to be checked
	 * @return true if a flag already has the flags for the this flag option
	 */
	std::pair<bool, std::string> hasFlagDashCaseInsen(const Flag& newFlag) const {
		auto noDashOpts = newFlag.flagsNoDash();
		strVecToLower(noDashOpts);
		for (const auto & f : flags_) {
			auto otherNoDashOtps = f.second.flagsNoDash();
			strVecToLower(otherNoDashOtps);
			for (const auto ndoPos : iter::range(noDashOpts.size())) {
				if (in(noDashOpts[ndoPos], otherNoDashOtps)) {
					return {true, noDashOpts[ndoPos]};
				}
			}
		}
		return {false, ""};
	}

	/**@brief Check to see if any of the other flags already have any of the flags for this flag option
	 *
	 * @param newFlag The flag to be checked
	 * @return true if a flag already has the flags for the this flag option
	 */
	void hasFlagDashCaseInsenThrow(const Flag& newFlag) const {
		auto noDashOpts = newFlag.flagsNoDash();
		strVecToLower(noDashOpts);
		for (const auto & f : flags_) {
			auto otherNoDashOtps = f.second.flagsNoDash();
			strVecToLower(otherNoDashOtps);
			for (const auto ndoPos : iter::range(noDashOpts.size())) {
				if (in(noDashOpts[ndoPos], otherNoDashOtps)) {
					std::stringstream ss;
					ss << "Error in : " << __PRETTY_FUNCTION__ << ", adding new flag: "
							<< noDashOpts[ndoPos] << " which already exists" << "\n";
					ss << json::writeAsOneLine(f.second.toJson()) << "\n";
					throw std::runtime_error { ss.str() };
				}
			}
		}
	}

	/**@brief convert to json representation
	 *
	 * @return Json::Value object
	 */
	Json::Value toJson() const {
		Json::Value ret;
		ret["class"] = getTypeName(*this);
		ret["flags_"] = njh::json::toJson(flags_);
		return ret;
	}

};

}  // namespace progutils
}  // namespace njh

