#pragma once
/*
 * sysUtils.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: nickhathaway
 */


#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/jsonUtils/jsonUtils.hpp"
#include "bibcpp/utils/timeUtils.hpp"
#include <pstreams/pstream.h>
#include <thread>
#include <mutex>
#include <atomic>

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

	Json::Value toJson()const {
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

/**@brief run the command in cmds externally and return the status and outputs
 *
 * @param cmds A vector of strings contains the command, content of cmds will be converted to a string with space delimited every string in cmds
 * @return A RunOutPut object holding status and outputs of the externally ran cmd
 * @todo add run time duration
 */
inline RunOutput run(std::vector<std::string> cmds) {
	std::string cmd = conToStr(cmds, " ");
	/*
	bib::stopWatch watch;
	std::cout << "cmd1: " << cmd << std::endl;;
	redi::ipstream s(cmd);
	std::cout << "cmd2: " << cmd << std::endl;;
	*/
	/*
	//read stdout
	std::stringstream outSS;
	outSS << s.out().rdbuf();
	auto out = outSS.str();
	trim(out);
	//read stderr
	std::stringstream errSS;
	errSS << s.err().rdbuf();
	auto err = errSS.str();
	trim(err);*/
/*
	s.close();
	double rTime = watch.totalTime();

	const int32_t errCode = s.rdbuf()->status();
	if (s.rdbuf()->exited() && !errCode) {
		return {true, 0, "", "", cmd, rTime};
	}

	return {false, errCode, "", "", cmd, rTime};
	*/
	bib::stopWatch watch;
	//std::cout << "cmd1: " << cmd << std::endl;;
	redi::ipstream s(cmd, redi::ipstream::pstdout | redi::ipstream::pstderr);
	//std::cout << "cmd2: " << cmd << std::endl;;
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

/**@brief Class to hold a pool of system commands with a lock so threads can just pull from this
 *
 */
template<typename T>
class CmdPool {
public:
	/**@brief Construct with a vector of commands to run
	 *
	 * @param cmds The vector of commands
	 */
	CmdPool(const std::vector<T> & cmds): cmds_(cmds){

	}
private:
	//std::atomic<uint32_t> currentCmd_;
	std::vector<T> cmds_; /**< The commands to run */
	uint32_t currentCmd_ = 0; /**< Index of what command to run next */

	std::mutex mut_; /**< Mutex for locking pool while thread is grabbing command */
public:
	/**@brief small class to hold whether there is a next class and what it is
	 *
	 */
	struct cmdStatus{
		T cmd_;
		bool valid_ ;
	};
	/**@brief grab next command to run
	 *
	 * @return A struct with whether there is a command to run and a string of what that command is, if invalid string will be blank
	 */
	cmdStatus getCmd(){
		/*
		auto current = currentCmd_.fetch_add(1);
		if (currentCmd_ < cmds_.size()) {
			return {cmds_ [current],true};
		} else {
			return {T {},false};
		}*/

		std::lock_guard<std::mutex> lock(mut_);
		if(currentCmd_ < cmds_.size()){
			++currentCmd_;
			return {cmds_ [currentCmd_ - 1],true};
		}else{
			return {T{},false};
		}
	}
};

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
	/*
	auto runCmds =
			[&verbose](const std::shared_ptr<CmdPool<std::string>> & pool) {
				bool running = true;
				while(running) {
					auto currentCmd = pool->getCmd();
					if(currentCmd.valid_) {
						if(verbose) {
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
						}
					} else {
						running = false;
					}
				}
			};*/
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


} // namespace sys
} // namespace bib





