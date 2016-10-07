#pragma once
//
//  commandLineArguments.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 9/29/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibcpp/utils.h"
#include "bibcpp/files/filePathUtils.hpp"
#include "bibcpp/bashUtils.h"


namespace bib {
namespace progutils {

/**@brief Convert the normal argc and argv arguments into a class enabled for flag parsing
 *searching
 *
 */
class CmdArgs {
public:
	/**@brief for determining if T is a type supported by convert command line argument
	 *
	 */
	template<typename T>
	struct is_cmdArg_supported_type : public std::integral_constant<bool,
	   std::is_same<std::string, typename std::decay<T>::type>::value
	|| std::is_same<boost::filesystem::path, typename std::decay<T>::type>::value
	|| std::is_same<bool, typename std::decay<T>::type>::value
	|| std::is_same<int, typename std::decay<T>::type>::value
	|| std::is_same<short, typename std::decay<T>::type>::value
	|| std::is_same<int, typename std::decay<T>::type>::value
	|| std::is_same<long, typename std::decay<T>::type>::value
	|| std::is_same<long long, typename std::decay<T>::type>::value
	|| std::is_same<unsigned short, typename std::decay<T>::type>::value
	|| std::is_same<unsigned int, typename std::decay<T>::type>::value
	|| std::is_same<unsigned long, typename std::decay<T>::type>::value
	|| std::is_same<double, typename std::decay<T>::type>::value
	|| std::is_same<long double, typename std::decay<T>::type>::value
	|| std::is_same<float, typename std::decay<T>::type>::value
	> {};

	/**@brief simply aesthetic, to make call to is_cmdArg_supported_type look nicer
	 *
	 * @return Returns true if T is of type arithmetic
	 */
	template<typename T>
	constexpr bool isSupportedType(){
		return is_cmdArg_supported_type<T>::value;
	}

	template<typename T>
	void convertArg(const std::string & arg, T & outVal) {
		static_assert(isSupportedType<T>(), "Type not supported in bib::progutils::CmdArgs::convertArg");
	}

	/**@brief Construct from the generic argc and argv c++ arguments
	 *
	 */
	CmdArgs(int argc, char* argv[]) :
			arguments_(parseArguments(argc, argv)) {
		masterProgram_ = files::bfs::path(argv[0]).filename().string();
		masterProgramRaw_ = argv[0];
		std::string secondArg = "";
		if (argc > 1) {
			secondArg = strToLowerRet(argv[1]);
		}
		if (!beginsWith(secondArg, "-")) {
			subProgram_ = secondArg;
			removeArgumentCaseInsen(subProgram_);
		}
		workingDir_ = files::get_cwd();
		commandLine_ = logRunArgumnets(argc, argv);
	}
	/**@brief Construct from a map of commands, key is flag and value is flag associated
	 *value
	 *
	 */
	CmdArgs(const std::string & masterProgram, const std::string & subProgram,
			const std::map<std::string, std::string>& inputCommands,
			const std::string & commandLine, const std::string & workingDir) :
			masterProgram_(masterProgram), masterProgramRaw_(masterProgram), subProgram_(
					subProgram), arguments_(inputCommands), commandLine_(commandLine), workingDir_(
					workingDir) {
		if ("" != subProgram_) {
			removeArgumentCaseInsen(subProgram_);
		}
	}

	/**@brief Name of the main Master Program being called, the name of the executable
	 *
	 */
	std::string masterProgram_;
	/**@brief Name of the main Master Program being called, the name of the executable
		 *
		 */
	std::string masterProgramRaw_;
	/**@brief Name of the subprogram if there is one
	 *
	 */
	std::string subProgram_;
	/**@brief A map of the commandline arguments, key is flag and value is flag
	 *associated value
	 *
	 */
	std::map<std::string, std::string> arguments_;
	/**@brief A string containing what the command was from the commandline and from what directory it was ran
	 *
	 */
	std::string commandLine_;

	/**@brief Directory from which the command was run
	 *
	 */
	std::string workingDir_;

	/**@brief return bool to indicated whether a help statement was requested
	 *
	 */
	bool printingHelp(){
		return hasFlagCaseInsenNoDash("-h") ||
						hasFlagCaseInsenNoDash("--help");
	}

	/**@brief return bool to indicated whether simply flags were requested
	 *
	 */
	bool gettingFlags(){
		return hasFlagCaseInsenNoDash("-getFlags");
	}

	/**@brief return bool to indicated whether simply to print version, verbose
	 *
	 */
	bool gettingVersion(){
		return hasFlagCaseInsenNoDash("--version");
	}

	/**@brief dumping version, just print version and nothing else
	 *
	 */
	bool gettingDumpVersion(){
		return hasFlagCaseInsenNoDash("--dumpversion");
	}

	/**@brief convert to json representation
	 *
	 * @return Json::Value object
	 */
	Json::Value toJson() const {
		Json::Value ret;
		ret["class"] = bib::json::toJson(getTypeName(*this));
		ret["masterProgram_"] = bib::json::toJson(masterProgram_);
		ret["masterProgramRaw_"] = bib::json::toJson(masterProgramRaw_);
		ret["subProgram_"] = bib::json::toJson(subProgram_);
		ret["arguments_"] = bib::json::toJson(arguments_);
		ret["commandLine_"] = bib::json::toJson(commandLine_);
		ret["workingDir_"] = bib::json::toJson(workingDir_);
		return ret;
	}

	/**@brief Get the program name, will add the subprogram if there is one
	 *
	 * @return The name of the program and possible sub-program
	 */
	std::string getProgramName() const {
		std::string ret = masterProgram_;
		if ("" != subProgram_) {
			ret += " " + subProgram_;
		}
		return ret;
	}

private:
	std::string getArgCaseInsen(std::string flag, bool boolOption) {
		std::string ret = "";
		std::string originalFlag = flag;
		strToLower(flag);
		bool found = false;
		for (const auto & arg : arguments_) {
			if (strToLowerRet(arg.first) == flag) {
				ret = arg.second;
				found = true;
				break;
			}
		}
		if (found) {
			if ("" == ret && !boolOption) {
				std::stringstream ss;
				ss << "Argument " << originalFlag
						<< " requires an option but none was given" << std::endl;
				throw std::runtime_error { ss.str() };
			}
		} else {
			std::stringstream ss;
			ss << "Argument " << flag << " wasn't found" << std::endl;
			throw std::runtime_error { ss.str() };
		}
		return ret;
	}

	std::string getArgCaseInsenNoDash(std::string flag, bool boolOption) {
		std::string ret = "";
		std::string originalFlag = flag;
		lstrip(flag, '-');
		strToLower(flag);
		bool found = false;
		for (const auto & arg : arguments_) {
			if (lstripRet(strToLowerRet(arg.first), '-') == flag) {
				ret = arg.second;
				found = true;
				break;
			}
		}
		if (found) {
			if ("" == ret && !boolOption) {
				std::stringstream ss;
				ss << "Argument " << originalFlag
						<< " requires an option but none was given" << std::endl;
				throw std::runtime_error { ss.str() };
			}
		} else {
			std::stringstream ss;
			ss << "Argument " << flag << " wasn't found" << std::endl;
			throw std::runtime_error { ss.str() };
		}
		return ret;
	}

public:
	/**@brief Look for options with ignoring case
	 *
	 * @param option the option to set associated with flag
	 * @param flag The flag string to look for
	 * @return whether the flag was found
	 */
	template<typename T>
	bool lookForOptionCaseInsen(T & option, const std::string& flag) {
		if (hasFlagCaseInsen(flag)) {

			convertArg(getArgCaseInsen(flag, getTypeName(option) == "bool"), option);
			return true;
		} else {
			return false;
		}
	}
	/**@brief Look for options with ignoring the number of dashes and case
	 *
	 * @param option the option to set associated with flag
	 * @param flag The flag string to look for
	 * @return whether the flag was found
	 */
	template<typename T>
	bool lookForOptionDashCaseInsen(T & option, const std::string& flag) {
		if (hasFlagCaseInsenNoDash(flag)) {
			convertArg(getArgCaseInsenNoDash(flag, getTypeName(option) == "bool"), option);
			return true;
		} else {
			return false;
		}
	}

	/** @brief Check to see if flag is present as is
	 *
	 * Checks for the present of a flag while being case sensitive
	 *
	 * @param flag The flag to be searched for for current option
	 * @return Bool, true if flag is present or false if flag is absent
	 */
	bool hasFlagCaseSens(const std::string& flag) {
		if (arguments_.find(flag) != arguments_.end()) {
			return true;
		} else {
			return false;
		}
	}
	/** @brief Check to see if flag is present as is
	 *
	 * Checks for the present of a flag while being case sensitive
	 *
	 * @param flag The flag to be searched for for current option
	 * @return Bool, true if flag is present or false if flag is absent
	 */
	bool hasFlagCaseSensNoDash(std::string flag) {
		lstrip(flag, '-');
		for (const auto & arg : arguments_) {
			if (lstripRet(arg.first, '-') == flag) {
				return true;
			}
		}
		return false;
	}

	/** @brief Check to see if flag, case insensitive
	 * Checks for the present of a flag ignoring case of letters
	 *
	 * @param flag The flag to be searched for for current option
	 * @return Bool, true if flag is present or false if flag is absent
	 */
	bool hasFlagCaseInsen(const std::string& flag) {
		for (const auto & arg : arguments_) {
			if (strToLowerRet(arg.first) == strToLowerRet(flag)) {
				return true;
			}
		}
		return false;
	}
	/** @brief Check to see if flag, case insensitive
	 * Checks for the present of a flag ignoring case of letters
	 *
	 * @param flag The flag to be searched for for current option
	 * @return Bool, true if flag is present or false if flag is absent
	 */
	bool hasFlagCaseInsenNoDash(std::string flag) {
		lstrip(flag, '-');
		strToLower(flag);
		for (const auto & arg : arguments_) {
			if('-' == arg.first.front()){
				if (lstripRet(strToLowerRet(arg.first), '-') == flag) {
					return true;
				}
			}
		}
		return false;
	}

	/**@brief Get the sub program in all lower case letters
	 *
	 * @return name of sub program in lower case
	 */
	std::string subProgramLowerCase() const {
		return strToLowerRet(subProgram_);
	}
	/**@brief Remove an arguments with no check on case
	 *
	 * @param arg The argument to remove
	 * @return Whether the argument was actually here
	 */
	bool removeArgumentCaseInsen(std::string flag) {
		bool hasArg = false;
		std::string argStored = "";
		strToLower(flag);
		for (const auto & arg : arguments_) {
			if (strToLowerRet(arg.first) == flag) {
				hasArg = true;
				argStored = arg.first;
			}
		}
		if (hasArg) {
			arguments_.erase(argStored);
		}
		return hasArg;
	}

	/**@brief Remove an arguments with no check on case
	 *
	 * @param arg The argument to remove
	 * @return Whether the argument was actually here
	 */
	bool removeArgumentCaseSense(const std::string & flag) {
		bool hasArg = arguments_.find(flag) != arguments_.end();
		if (hasArg) {
			arguments_.erase(flag);
		}
		return hasArg;
	}

	/**@brief Get a string of the input arguments as a string with arguments surrounded by quotes in case they contain spaces
	 *
	 * @param argc number of arguments
	 * @param argv array of arguments
	 * @return
	 */
	static std::string logRunArgumnets(int argc, char* argv[]) {
		std::stringstream ss;
		std::string secondArg = "";
		if (argc > 1) {
			secondArg = strToLowerRet(argv[1]);
		}
		for (int args = 0; args < argc; ++args) {
			if (args == 0) {
				ss << argv[args];
			} else if (args == 1 && !beginsWith(secondArg, "-")) {
				ss << argv[args];
			} else {
				if(strHasWhitesapce(argv[args])){
					ss << "\"" << argv[args] << "\"";
				}else{
					ss << argv[args];
				}
			}
			if (args + 1 != argc) {
				ss << " ";
			}
		}
		ss << std::endl;
		return ss.str();
	}

	/**@brief Log run arguments already stored, won't always perfectly capture original run arguments
	 *
	 * @return A string of the run arguments
	 */
	std::string logRunArgumnets() const {
		std::stringstream ss;
		ss << masterProgram_;
		if ("" != subProgram_) {
			ss << " " << subProgram_;
		}
		for (const auto & arg : arguments_) {
			ss << " ";
			if ("" == arg.second) {
				ss << arg.first;
			} else {
				if (beginsWith(arg.second, "-")) {
					if(strHasWhitesapce(arg.first) || strHasWhitesapce(arg.second)){
						ss << "\"" << arg.first << "=" << arg.second << "\"";
					}else{
						ss << arg.first << "=" << arg.second;
					}
				} else {
					ss << arg.first << " ";
					if(strHasWhitesapce(arg.second)){
						ss << "\""<< arg.second << "\"";
					}else{
						ss << arg.second;
					}
				}
			}
		}
		return ss.str();
	}

	/**@brief Reset the command line call with what is currently stored in arguments
	 *
	 */
	void resetCommandLine() {
		commandLine_ = logRunArgumnets();
	}

	/**@brief Write the directory ran from and the arguments used
	 *
	 * @param out
	 */
	void stampWithArgsAndWorkDir(std::ostream& out) {
		out << "Command ran from " << std::endl;
		out << workingDir_ << std::endl;
		out << commandLine_ << std::endl;
	}

	/** @brief Static function converting commandline arguments to a map of
	 *string, string pairs
	 * Converts command line arguments to a map of key flag and value of flag
	 *associated value
	 * Will check for double arguments and arugments can be given as -flag value
	 *or -flag=value
	 *
	 * @param argc The number of arguments
	 * @param argv The array of char pointers of the arguments
	 * @return A map of string string pairs
	 */
	static std::map<std::string, std::string> parseArguments(int argc,
			char* argv[]) {
		std::map<std::string, std::string> storage;
		for (int i = 1; i < argc; i++) {
			std::string nextParamRaw = argv[i];
			uint32_t equCount = std::count(nextParamRaw.begin(), nextParamRaw.end(),
					'=');
			if (equCount > 0) {
				if (equCount > 1) {
					std::stringstream ss;
					ss << bashCT::addColor(196) << std::endl;
					ss << "Error in: " << __PRETTY_FUNCTION__ << std::endl;
					ss << "Error can only have one equal sign when setting flag"
							<< std::endl;
					ss << "eg. -flag=this , can't have -flag=this=that" << std::endl;
					ss << "not " << nextParamRaw << bashCT::reset << std::endl;
					throw std::runtime_error { ss.str() };
				} else {
					std::vector<std::string> toks = tokenizeString(nextParamRaw, "=");
					storage.insert(std::make_pair(strToLowerRet(toks[0]), toks[1]));
				}
			} else {
				std::string nextParam = strToLowerRet(argv[i]);
				if (storage.find(nextParam) != storage.end()) {
					std::stringstream ss;
					std::cout << "Error, already have " << nextParam << std::endl;
					std::cout << "Check if you entered it in twice" << std::endl;
					throw std::runtime_error { ss.str() };
				}

				if (nextParam.front() == '-') {
					uint32_t pos = 0;
					uint32_t dashCount = 0;
					while(pos < nextParam.length() && nextParam[pos] == '-'){
						++dashCount;
						++pos;
					}
					if(dashCount > 2){
						std::stringstream ss;
						std::cout << "Error, flags should only start with 1 or 2 dashes, not " << dashCount << ", " << nextParam << "\n";
						throw std::runtime_error { ss.str() };
					}
					if(dashCount == 1){
						if (storage.find("-" + nextParam) != storage.end()) {
							std::stringstream ss;
							std::cout << "Error, already have " << nextParam << " as " << "-" << nextParam<< std::endl;
							std::cout << "Check if you entered it in twice" << std::endl;
							throw std::runtime_error { ss.str() };
						}
					}else if(dashCount == 2){
						if (storage.find(nextParam.substr(1)) != storage.end()) {
							std::stringstream ss;
							std::cout << "Error, already have " << nextParam << " as " << nextParam.substr(1) << std::endl;
							std::cout << "Check if you entered it in twice" << std::endl;
							throw std::runtime_error { ss.str() };
						}
					}
					if (i + 1 >= argc) {
						storage.insert(std::make_pair(strToLowerRet(nextParam), ""));
					} else {
						if (argv[i + 1][0] == '-') {
							storage.insert(std::make_pair(strToLowerRet(nextParam), ""));
						} else {
							storage.insert(
									std::make_pair(strToLowerRet(nextParam), argv[i + 1]));
							++i;
						}
					}
				} else {
					storage.insert(std::make_pair(strToLowerRet(nextParam), ""));
				}
			}
		}
		return storage;
	}
	/** @brief Static function converting commandline arguments to a map of
	 *string, string pairs
	 * Converts command line arguments to a map of key flag and value of flag
	 *associated value
	 * Will check for double arguments and arugments can be given as -flag value
	 *or -flag=value
	 *
	 * @param argc The number of arguments
	 * @param argv The array of char pointers of the arguments
	 * @return A map of string string pairs
	 */
	static std::map<std::string, std::string> parseArgumentsPreserveCase(int argc,
			char* argv[]) {
		std::map<std::string, std::string> storage;
		for (int i = 1; i < argc; i++) {
			std::string nextParamRaw = argv[i];
			uint32_t equCount = std::count(nextParamRaw.begin(), nextParamRaw.end(),
					'=');
			if (equCount > 0) {
				if (equCount > 1) {
					std::stringstream ss;
					ss << bashCT::addColor(196) << std::endl;
					ss << "Error in: " << __PRETTY_FUNCTION__ << std::endl;
					ss << "Error can only have one equal sign when setting flag"
							<< std::endl;
					ss << "eg. -flag=this , can't have -flag=this=that" << std::endl;
					ss << "not " << nextParamRaw << bashCT::reset << std::endl;
					throw std::runtime_error { ss.str() };
				} else {
					std::vector<std::string> toks = tokenizeString(nextParamRaw, "=");
					storage.insert(std::make_pair(toks[0], toks[1]));
				}
			} else {
				std::string nextParam = argv[i];
				if (storage.find(nextParam) != storage.end()) {
					std::stringstream ss;
					std::cout << "Error, already have " << nextParam << std::endl;
					std::cout << "Check if you entered it in twice" << std::endl;
					throw std::runtime_error { ss.str() };
				}

				if (argv[i][0] == '-') {
					uint32_t pos = 0;
					uint32_t dashCount = 0;
					while(pos < nextParam.length() && nextParam[pos] == '-'){
						++dashCount;
						++pos;
					}
					if(dashCount > 2){
						std::stringstream ss;
						std::cout << "Error, flags should only start with 1 or 2 dashes, not " << dashCount << ", " << nextParam << "\n";
						throw std::runtime_error { ss.str() };
					}
					if(dashCount == 1){
						if (storage.find("-" + nextParam) != storage.end()) {
							std::stringstream ss;
							std::cout << "Error, already have " << nextParam << " as " << "-" << nextParam << std::endl;
							std::cout << "Check if you entered it in twice" << std::endl;
							throw std::runtime_error { ss.str() };
						}
					}else if(dashCount == 2){
						if (storage.find(nextParam.substr(1)) != storage.end()) {
							std::stringstream ss;
							std::cout << "Error, already have " << nextParam << " as " << nextParam.substr(1) << std::endl;
							std::cout << "Check if you entered it in twice" << std::endl;
							throw std::runtime_error { ss.str() };
						}
					}
					if (i + 1 >= argc) {
						storage.insert(std::make_pair(nextParam, ""));
					} else {
						if (argv[i + 1][0] == '-') {
							storage.insert(std::make_pair(nextParam, ""));
						} else {
							storage.insert(std::make_pair(nextParam, argv[i + 1]));
							++i;
						}
					}
				} else {
					storage.insert(std::make_pair(nextParam, ""));
				}
			}
		}
		return storage;
	}
	/** @brief Get the number of arguments
	 *Gets the number of arguments while ignoring -program and -commandline as not
	 *being arguments
	 * @return The number of arguments
	 */
	uint32_t numberOfCommands() {
		return arguments_.size();
	}
};

// boost::filesystem::path
template<>
inline void CmdArgs::convertArg(const std::string& option,
		boost::filesystem::path & outVal) {
	outVal = option;
}

// std::string
template<>
inline void CmdArgs::convertArg(const std::string& option,
		std::string & outVal) {
	outVal = option;
}
// bool
template<>
inline void CmdArgs::convertArg(const std::string& option, bool & outVal) {
	if ("no" == option) {
		outVal = false;
	} else {
		outVal = true;
	}
}
// short (int16_t)
template<>
inline void CmdArgs::convertArg(const std::string& option, short & outVal) {
	outVal = estd::stos(option);
}
// int (int32_t)
template<>
inline void CmdArgs::convertArg(const std::string& option, int& outVal) {
	outVal = std::stoi(option);
}
// long (int64_t depending on system/lib)
template<>
inline void CmdArgs::convertArg(const std::string& option, long & outVal) {
	outVal = std::stol(option);
}

// long long (int64_t depending on system/lib)
template<>
inline void CmdArgs::convertArg(const std::string& option, long long& outVal) {
	outVal = std::stoll(option);
}
// unsigned short (uint16_t)
template<>
inline void CmdArgs::convertArg(const std::string& option,
		unsigned short& outVal) {
	outVal = estd::stous(option);
}
// unsigned int (uint32_t)
template<>
inline void CmdArgs::convertArg(const std::string& option,
		unsigned int& outVal) {
	outVal = estd::stou(option);
}
// unsigned long (size_t and on some systems/libs uint64_t)
template<>
inline void CmdArgs::convertArg(const std::string& option,
		unsigned long& outVal) {
	outVal = std::stoul(option);
}

// unsigned long long (uint64_t depending on the system/lib)
template<>
inline void CmdArgs::convertArg(const std::string& option,
		unsigned long long& outVal) {
	outVal = std::stoull(option);
}
// double
template<>
inline void CmdArgs::convertArg(const std::string& option, double& outVal) {
	outVal = std::stod(option);
}
// long double
template<>
inline void CmdArgs::convertArg(const std::string& option,
		long double& outVal) {
	outVal = std::stold(option);
}
// float
template<>
inline void CmdArgs::convertArg(const std::string& option, float& outVal) {
	outVal = std::stof(option);
}
}  // namespace progutils
}  // namespace bib
