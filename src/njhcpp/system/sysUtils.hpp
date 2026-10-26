#pragma once
/*
 * sysUtils.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: nickhathaway
 */


#include "njhcpp/utils/stringUtils.hpp"
#include "njhcpp/bashUtils/textFormatter.hpp"

#include "njhcpp/utils/time.h"
#include "njhcpp/system/CmdPool.hpp"
#include "njhcpp/system/RunOutput.hpp"
#include "njhcpp/concurrency/concurrencyUtils.hpp"
#include <pstreams/pstream.h>
#include <thread>
#include <mutex>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace njh{
namespace sys{

/**@brief guards creating the pipes and forking in run() so a command started on another thread can't inherit this command's pipes
 *
 */
inline std::mutex & runForkMutex() {
	static std::mutex mut;
	return mut;
}

/**@brief run the command in cmds externally with /bin/sh -c and return the status and outputs
 *
 * stdout and stderr are read together as they're written, so a command writing a lot to either can't block on a full pipe
 *
 * @param cmds A vector of strings contains the command, content of cmds will be converted to a string with space delimited every string in cmds
 * @return A RunOutPut object holding status and outputs of the externally ran cmd, returnCode_ is the exit code (128 + the signal number if the command was killed by a signal)
 */
inline RunOutput run(std::vector<std::string> cmds) {
	//cat cmds
	std::string cmd = conToStr(cmds, " ");
	//start a stopwatch to clock how long it took
	njh::stopWatch watch;
	int outPipe[2] = {-1, -1};
	int errPipe[2] = {-1, -1};
	pid_t pid = -1;
	{
		std::lock_guard<std::mutex> lock(runForkMutex());
		if (0 != ::pipe(outPipe) || 0 != ::pipe(errPipe)) {
			for (int fd : { outPipe[0], outPipe[1], errPipe[0], errPipe[1] }) {
				if (fd >= 0) {
					::close(fd);
				}
			}
			return {false, -1, "", std::string("failed to create pipes: ") + std::strerror(errno), cmd, watch.totalTime()};
		}
		for (int fd : { outPipe[0], outPipe[1], errPipe[0], errPipe[1] }) {
			::fcntl(fd, F_SETFD, FD_CLOEXEC);
		}
		pid = ::fork();
		if (0 == pid) {
			//child, only async-signal-safe calls until exec
			::dup2(outPipe[1], STDOUT_FILENO);
			::dup2(errPipe[1], STDERR_FILENO);
			::execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char *>(nullptr));
			::_exit(127);
		}
		::close(outPipe[1]);
		::close(errPipe[1]);
	}
	if (pid < 0) {
		::close(outPipe[0]);
		::close(errPipe[0]);
		return {false, -1, "", std::string("failed to fork: ") + std::strerror(errno), cmd, watch.totalTime()};
	}
	std::string outRaw;
	std::string errRaw;
	{
		struct pollfd fds[2] = { { outPipe[0], POLLIN, 0 }, { errPipe[0], POLLIN, 0 } };
		std::string * dests[2] = { &outRaw, &errRaw };
		uint32_t stillOpen = 2;
		char buf[64 * 1024];
		while (stillOpen > 0) {
			if (::poll(fds, 2, -1) < 0) {
				if (EINTR == errno) {
					continue;
				}
				break;
			}
			for (uint32_t pos = 0; pos < 2; ++pos) {
				if (fds[pos].fd < 0 || 0 == fds[pos].revents) {
					continue;
				}
				auto n = ::read(fds[pos].fd, buf, sizeof(buf));
				if (n > 0) {
					dests[pos]->append(buf, n);
				} else if (0 == n || EINTR != errno) {
					::close(fds[pos].fd);
					//poll ignores negative fds
					fds[pos].fd = -1;
					--stillOpen;
				}
			}
		}
		for (const auto & fd : fds) {
			if (fd.fd >= 0) {
				::close(fd.fd);
			}
		}
	}
	int status = 0;
	while (::waitpid(pid, &status, 0) < 0 && EINTR == errno) {
	}
	auto out = bashCT::trimForNonTerminalOut(outRaw);
	trim(out);
	auto err = bashCT::trimForNonTerminalOut(errRaw);
	trim(err);
	double rTime = watch.totalTime();
	int32_t exitCode = -1;
	if (WIFEXITED(status)) {
		exitCode = WEXITSTATUS(status);
	} else if (WIFSIGNALED(status)) {
		exitCode = 128 + WTERMSIG(status);
	}
	return {0 == exitCode && WIFEXITED(status), exitCode, std::move(out), std::move(err), cmd, rTime};
}

inline RunOutput runTest(std::vector<std::string> cmds) {
	//cat cmds
	std::string cmd = conToStr(cmds, " ");
	//start a stopwatch to clock how long it took
	njh::stopWatch watch;
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
 * @return A vector njh::sys::RunOutput for status of the commands
 */
inline std::vector<njh::sys::RunOutput> runCmdsThreaded(
		const std::vector<std::string> & cmds, uint32_t numThreads, bool verbose,
		bool debug) {
	std::mutex allOutputsMut;
	std::mutex coutMut;
	std::vector<njh::sys::RunOutput> ret;
	auto pool = std::make_shared<CmdPool<std::string>>(cmds);

	std::function<void()> runCmds =
			[&coutMut, &allOutputsMut, &ret, &verbose,&pool]() {
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
						auto currentOut = njh::sys::run(std::vector<std::string> {currentCmd.cmd_});
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

	njh::concurrent::runVoidFunctionThreaded(runCmds, numThreads);
	return ret;
}


/**@brief return a bool of the success of calling system which on a command
 *
 * @param cmd the command to look for
 * @return a bool whether calling which on the command name was successful
 */
inline bool hasSysCommand(const std::string & cmd){
	auto output = njh::sys::run({"which", cmd});
	return output.success_;
}

/**@brief return a the full output of calling system which on a command
 *
 * @param cmd the command to look for
 * @return the run output of the call to which, this way the path name and such can be recovered as well
 */
inline njh::sys::RunOutput hasSysCommandFullOut(const std::string & cmd){
	auto output = njh::sys::run({"which", cmd});
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
	auto hasProgram = njh::sys::hasSysCommand(program);
	if (!hasProgram) {
		std::stringstream ss;
		if (stdoutTerminal()) {
			ss << njh::bashCT::boldBlack(program)
					<< njh::bashCT::boldRed(
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
		auto hasProgram = njh::sys::hasSysCommand(program);
		if (!hasProgram) {
			failed = true;
			if (stdoutTerminal()) {
				ss << njh::bashCT::boldBlack(program)
						<< njh::bashCT::boldRed(
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
} // namespace njh





