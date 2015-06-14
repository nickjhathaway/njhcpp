#pragma once
/*
 * sysUtils.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: nickhathaway
 */


#include "bibcpp/utils/stringUtils.hpp"
#include <pstreams/pstream.h>

namespace bib {
namespace sys {

/**@b struct for holding output and success status of an external command
 *@todo add original run command and run duration info
 */
struct RunOutput {
	bool success_; /**< whether the command was successful*/
	int32_t returnCode_; /**< the return code of the command*/
	std::string stdOut_; /**< the output to stdout from the command*/
	std::string stdErr_; /**< the output to stderr from the command*/

	/**@b So the struct can be tested in an if statement
	 *
	 */
	explicit operator bool() const {
		return success_;
	}
};

/**@b run the command in cmds externally and return the status and outputs
 *
 * @param cmds A vector of strings contains the command, content of cmds will be converted to a string with space delimited every string in cmds
 * @return A RunOutPut object holding status and outputs of the externally ran cmd
 * @todo add run time duration
 */
inline RunOutput run(std::vector<std::string> cmds) {
	std::string cmd = conToStr(cmds, " ");
	//std::cout << "cmd: " << cmd << "\n";
	redi::ipstream s(cmd, redi::ipstream::pstdout | redi::ipstream::pstderr);

	//read stdout
	std::stringstream outSS;
	outSS << s.out().rdbuf();
	auto out = outSS.str();
	trim(out);
	//read stderr
	std::stringstream errSS;
	errSS << s.err().rdbuf();
	auto err = errSS.str();
	trim(err);

	s.close();

	const int32_t errCode = s.rdbuf()->status();
	if (s.rdbuf()->exited() && !errCode) {
		return {true, 0, std::move(out), std::move(err)};
	}

	return {false, errCode, std::move(out), std::move(err)};
}

} // namespace sys
} // namespace bib





