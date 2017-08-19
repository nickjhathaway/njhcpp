#pragma once
/*
 * sysUtils.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: nickhathaway
 */


#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/utils/time.h"
#include "bibcpp/system/CmdPool.hpp"
#include "bibcpp/system/RunOutput.hpp"
#include <pstreams/pstream.h>
#include <thread>

namespace bib{
namespace sys{

/**@brief run the command in cmds externally and return the status and outputs
 *
 * @param cmds A vector of strings contains the command, content of cmds will be converted to a string with space delimited every string in cmds
 * @return A RunOutPut object holding status and outputs of the externally ran cmd
 * @todo add run time duration
 */
inline RunOutput runOld(std::vector<std::string> cmds) {
	//cat cmds
	std::string cmd = conToStr(cmds, " ");
	//start a stopwatch to clock how long it took
	bib::stopWatch watch;
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
	double rTime = watch.totalTime();
	const int32_t errCode = s.rdbuf()->status();
	if (s.rdbuf()->exited() && !errCode) {
		return {true, 0, std::move(out), std::move(err), cmd, rTime};
	}
	return {false, errCode, std::move(out), std::move(err), cmd, rTime};
}

inline RunOutput run(std::vector<std::string> cmds) {
	//cat cmds
	std::string cmd = conToStr(cmds, " ");
	//start a stopwatch to clock how long it took
	bib::stopWatch watch;
	redi::ipstream s(cmd, redi::ipstream::pstdout | redi::ipstream::pstderr);
	//read stdout
	std::stringstream outSS;
	//read stderr
	std::stringstream errSS;
	char buf[1024];
	std::streamsize n;
	bool finished[2] = { false, false };
	while (!finished[0] || !finished[1]) {
		if (!finished[0]) {
			while ((n = s.err().readsome(buf, sizeof(buf))) > 0) {
				errSS.write(buf, n);
			}
			if (s.eof()) {
				finished[0] = true;
				if (!finished[1]) {
					s.clear();
				}
			}
		}
		if (!finished[1]) {
			while ((n = s.out().readsome(buf, sizeof(buf))) > 0) {
				outSS.write(buf, n).flush();
			}
			if (s.eof()) {
				finished[1] = true;
				if (!finished[0]) {
					s.clear();
				}
			}
		}
	}
	auto out = outSS.str();
	trim(out);

	auto err = errSS.str();
	trim(err);
	s.close();

	double rTime = watch.totalTime();
	const int32_t errCode = s.rdbuf()->status();
	if (s.rdbuf()->exited() && !errCode) {
		return {true, 0, std::move(out), std::move(err), cmd, rTime};
	}
	return {false, errCode, std::move(out), std::move(err), cmd, rTime};
}





/**@brief Run a vector of commands on multiple threads
 * Run multiple commands on the system in several threads, no safety checks on number of threads available or
 *  if commands would clash.  Will run all commands even if one fails, intention of this command is to run
 *  a bunch of small jobs at once
 *
 * @param cmds A vector of commands to run in parallel, no check is done to ensure they are not clashing
 * @param numThreads The number of threads to use
 * @param verbose Whether to be print the command when it is being run
 * @param debug Whether to just print the cmds and exit
 * @return A vector bib::sys::RunOutput for status of the commands
 */
inline std::vector<bib::sys::RunOutput> runCmdsThreaded(
		const std::vector<std::string> & cmds, uint32_t numThreads, bool verbose,
		bool debug) {
	std::mutex allOutputsMut;
	std::mutex coutMut;
	std::vector<bib::sys::RunOutput> ret;

	auto runCmds =
			[&coutMut, &allOutputsMut, &ret, &verbose](const std::shared_ptr<CmdPool<std::string>> & pool) {
				bool running = true;
				while(running) {
					auto currentCmd = pool->getCmd();
					if(currentCmd.valid_) {
						if(verbose) {
							std::lock_guard<std::mutex> lock(coutMut);
							auto tId = std::this_thread::get_id();
							std::cout << "Thread: " << tId << std::endl;
							std::cout << "\tRunning: " << currentCmd.cmd_ << std::endl;
						}
						auto currentOut = bib::sys::run(std::vector<std::string> {currentCmd.cmd_});
						if(verbose){
							auto tId = std::this_thread::get_id();
							std::cout << "\tThread: " << tId << std::endl;
							std::cout << "\tDone running: " << currentCmd.cmd_ << std::endl;
						}
						{
							if(verbose){
								auto tId = std::this_thread::get_id();
								std::cout << "\tThread: " << tId << std::endl;
								std::cout << "\tInserting Results from: " << currentCmd.cmd_ << std::endl;
							}
							std::lock_guard<std::mutex> lock(allOutputsMut);
							ret.emplace_back(currentOut);
						}
					} else {
						running = false;
					}
				}
			};
	if (debug) {
		for (const auto & cmd : cmds) {
			std::cout << cmd << std::endl;
		}
		exit(1);
	}
	auto pool = std::make_shared<CmdPool<std::string>>(cmds);
	std::vector<std::thread> threads;
	for (uint32_t t = 0; t < numThreads; ++t) {
		threads.emplace_back(std::thread(runCmds, std::cref(pool)));
	}
	for (auto &t : threads) {
		t.join();
	}
	return ret;
}


/**@brief return a bool of the success of calling system which on a command
 *
 * @param cmd the command to look for
 * @return a bool whether calling which on the command name was successful
 */
inline bool hasSysCommand(const std::string & cmd){
	auto output = bib::sys::run({"which", cmd});
	return output.success_;
}

/**@brief return a the full output of calling system which on a command
 *
 * @param cmd the command to look for
 * @return the run output of the call to which, this way the path name and such can be recovered as well
 */
inline bib::sys::RunOutput hasSysCommandFullOut(const std::string & cmd){
	auto output = bib::sys::run({"which", cmd});
	return output;
}


/**@brief Check to see if bash environment has variable
 *
 * @param var The variable to check for
 * @return whether or not the environment contains var
 */
inline bool EnvHasVar(const std::string & var) {
	const char * val = ::getenv(var.c_str());
	if (0 == val) {
		return false;
	} else {
		return true;
	}
}

/**@brief Get the value for the environmental variable
 *
 * @param var The variable to get the value of
 * @return the value held in the var
 */
inline std::string GetEnv(const std::string & var) {
	const char * val = ::getenv(var.c_str());
	if (0 == val) {
		return "";
	} else {
		return val;
	}
}

/**@brief Test if std::cout is going to terminal or being redirected
 *
 * @return true if going to terminal
 */
inline bool stdoutTerminal(){
	return isatty(STDOUT_FILENO);
}

/**@brief Test if std::cin is from terminal or being redirected
 *
 * @return true if coming from terminal
 */
inline bool stdinTerminal(){
	return isatty(STDIN_FILENO);
}


/**@brief Throw if an external program isn't available
 *
 * @param program the program to check for
 * @return true if has program ... which i guess is kind of pointless since it would have thrown anyways
 */
inline bool requireExternalProgramThrow(const std::string & program) {
	auto hasProgram = bib::sys::hasSysCommand(program);
	if (!hasProgram) {
		std::stringstream ss;
		if (stdoutTerminal()) {
			ss << bib::bashCT::boldBlack(program)
					<< bib::bashCT::boldRed(
							" is not in path or may not be executable, cannot be used")
					<< std::endl;
		} else {
			ss << program
					<< " is not in path or may not be executable, cannot be used"
					<< std::endl;
		}
		throw std::runtime_error { ss.str() };
	}
	return hasProgram;
}


/**@brief Throw if any of the external programs aren't available
 *
 * @param programs the programs to check for
 * @return true if has program ... which i guess is kind of pointless since it would have thrown anyways
 */
inline bool requireExternalProgramsThrow(
		const std::vector<std::string> & programs) {
	bool failed = false;
	std::stringstream ss;
	for (const auto & program : programs) {
		auto hasProgram = bib::sys::hasSysCommand(program);
		if (!hasProgram) {
			failed = true;
			if (stdoutTerminal()) {
				ss << bib::bashCT::boldBlack(program)
						<< bib::bashCT::boldRed(
								" is not in path or may not be executable, cannot be used")
						<< std::endl;
			} else {
				ss << program
						<< " is not in path or may not be executable, cannot be used"
						<< std::endl;
			}
		}
	}

	if (failed) {
		throw std::runtime_error { ss.str() };
	}
	return !failed;
}

} // namespace sys
} // namespace bib





