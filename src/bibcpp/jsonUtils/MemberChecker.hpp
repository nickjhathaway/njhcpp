#pragma once
/*
 * MemberChecker.hpp
 *
 *  Created on: Sep 11, 2016
 *      Author: nick
 */



#include "bibcpp/jsonUtils/jsonUtils.hpp"
#include "bibcpp/utils/stringUtils.hpp" //bib::conToStr

namespace bib {

namespace json {

/**@brief Simple class to check to see if a json object contains members
 *
 */
class MemberChecker {
public:

	/**@brief construct with reference to json object
	 *
	 * @param value the json object to check
	 */
	MemberChecker(const Json::Value & value) :
			value_(value) {
	}

	const Json::Value & value_; /**< a const ref to the json object to check */
	bool fail_ = false; /**< whether the check has failed*/
	std::stringstream message_; /**< the message for the failure if any */

	/**@brief check to see if value_ has the following members
	 *
	 * @param memberNames the members to check for
	 * @param funcName the function from the which the function is being called, normally just __PRETTY_FUNCTION__
	 * @return true if at least one of the members are missing
	 */
	bool failMemberCheck(const std::vector<std::string> & memberNames,
			const std::string & funcName) {

		for (const auto & memberName : memberNames) {
			if (!value_.isMember(memberName)) {
				message_ << funcName << ": error, input json value needs to have "
						<< memberName << "\n";
				fail_ = true;
			}
		}
		return fail_;
	}

	/**@brief same as failMemberCheck but will throw an exception if there is failure
	 *
	 * @param memberNames the member names to check
	 * @param funcName the function from the which the function is being called, normally just __PRETTY_FUNCTION__
	 * @return true if at least one of the members are missing
	 */
	bool failMemberCheckThrow(const std::vector<std::string> & memberNames,
			const std::string & funcName) {
		failMemberCheck(memberNames, funcName);
		if (fail_) {
			message_ << "members are: "
					<< bib::conToStr(value_.getMemberNames(), ", ") << "\n";
			throw std::runtime_error { message_.str() };
		}
		return fail_;
	}

};


}  // namespace json

}  // namespace bib

