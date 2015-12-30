#pragma once
/*
 * RunOutput.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */

#include "bibcpp/common.h"
#include "bibcpp/jsonUtils/jsonUtils.hpp"

namespace bib {
namespace sys {

/**@brief struct for holding output and success status of an external command
 *@todo add original run command and run duration info
 */
struct RunOutput {
	bool success_; /**< whether the command was successful*/
	int32_t returnCode_; /**< the return code of the command*/
	std::string stdOut_; /**< the output to stdout from the command*/
	std::string stdErr_; /**< the output to stderr from the command*/
	std::string cmd_; /**< the command*/
	double time_; /**< time in millisecond */

	/**@brief So the struct can be tested in an if statement
	 *
	 */
	explicit operator bool() const {
		return success_;
	}

	Json::Value toJson() const {
		Json::Value ret;
		ret["class"] = "bib::sys::RunOutput";
		ret["cmd_"] = json::toJson(cmd_);
		ret["succes_"] = json::toJson(success_);
		ret["returnCode_"] = json::toJson(returnCode_);
		ret["stdOut_"] = json::toJson(stdOut_);
		ret["stdErr_"] = json::toJson(stdErr_);
		ret["time_"] = json::toJson(time_);
		return ret;
	}
};
}  // namespace sys
}  // namespace bib

