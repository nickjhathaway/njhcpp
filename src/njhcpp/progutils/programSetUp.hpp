#pragma once
//
//  programSetUp.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 9/27/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "njhcpp/utils.h"
#include "njhcpp/bashUtils.h"
#include "njhcpp/common.h"
#include "njhcpp/files/fileStreamUtils.hpp"
#include "njhcpp/progutils/runLog.hpp"
#include "njhcpp/progutils/FlagHolder.hpp"
#include "njhcpp/progutils/CmdArgs.hpp"

namespace njh {
/**@brief Namespace for dealing with program option parsing and running sub programs
 *
 */
namespace progutils {

/**@brief Class for helping set up current program with flag parsing, logging run
 *times, getting help messages
 *
 */
class ProgramSetUp {

public:
	/**@brief Construct the setUp with the generic argc and argv
	 *
	 * @param argc The number of arguments
	 * @param argv The array of char pointers of the arguments
	 *
	 */
	ProgramSetUp(int argc, char *argv[]) :
			commands_(CmdArgs(argc, argv)) {
		init();
	}

	/**@brief Construct the setUp with the commandLineArguments class converted
	 *from argc and argv
	 *
	 * @param inputCommands The commandLineArguments object likely converted from
	 *argc and argv
	 *
	 */
	ProgramSetUp(const CmdArgs &inputCommands) :
			commands_(inputCommands) {
		init();
	}

	/**@brief Initialize any defaults
	 *
	 */
	void init() {
		if (commands_.gettingVersion()
				|| commands_.gettingDumpVersion()
				|| commands_.gettingFlags()
				|| commands_.printingHelp()) {
			failed_ = true;
		}
	}


	/**@brief A commandLineArguments class to store the objects and help with flag
	 *parsing, see commandLineArguments for more details
	 *
	 */
	CmdArgs commands_;

	/**@brief A class using the std::chrono library to log infomration about run times
	 *
	 *
	 */
	stopWatch timer_;

	std::string description_;/**< A short description of the program, gets printed with --getFlags*/

	std::vector<std::string> examples_;/**< A few examples of running, gets printed with --getFlags*/

protected:

	/**@brief A vector of strings of warnings to be printed at the end of setUp
	 *
	 */
	std::vector<std::string> warnings_;

public:
	/**@brief A bool to indicated whether the set up has failed somewhere
	 *
	 */
	bool failed_ = false;



	/**@brief The maximum width to allow for messages
	 *
	 */
	uint32_t width_ = 80;

	/**@brief The indent amount for wrapping messages
	 *
	 */
	uint32_t indent_ = 4;

	/**@brief A container for parameters set during set up, see parametersHolder
	 *for more details
	 *
	 */
	FlagHolder flags_;

	/**@brief A runLog object for optional logging of run time info, won't be
	 *started unless programSetUp::startARunLog is called
	 *
	 */
	RunLog rLog_;

	/**@brief A function to start a runLog in the named directory
	 * @param dirName The name of the directory to start the runLog, runLog name
	 *will be runLog_[NAME_OF_PROGRAM]
	 *
	 */
	void startARunLog(const std::string &dirName) {
		rLog_.setFilenameAndOpen(
				files::make_path(dirName,"runLog_"
						+ replaceString(replaceString(commands_.getProgramName(), "./", ""),
								" ", "-")+ "_" + getCurrentDate() + ".txt").string(), timer_.start_);
		rLog_.startRunLog(commands_);
	}

	/**@brief A function to write info on the parameters set during the set up
	 * process
	 * @param fileName The name of the parameter
	 * @param overWrite A bool indicating whether to overwrite any file with that
	 * name
	 * @param failOnWriteFailure A bool whether to exit if the parameters file
	 * couldn't be written
	 */
	void writeParametersFile(const std::string &fileName, bool overWrite,
			bool failOnWriteFailure) {
		std::ofstream parameterFile;
		std::ostream out(
				files::determineOutBuf(parameterFile, fileName, ".txt", overWrite,
						false, failOnWriteFailure));
		//parameterFile << commands_.commandLine_ << std::endl;
		flags_.outputParsFile(out);
	}

	/**@brief Called at the end of set up to look for any invalid options
	 *
	 */
	void lookForInvalidOptions() {
		auto flagStrs = flags_.getFlags();
		strVecToLower(flagStrs);
		for (const auto &com : commands_.arguments_) {
			if (!contains(flagStrs, com.first)) {
				warnings_.emplace_back(
						bashCT::bold + bashCT::red + "Unrecognized option, " + com.first
								+ " not using" + bashCT::reset);
				failed_ = true;
			}
		}
	}

	/**@brief Called at the end of set up to look for any invalid options, ignoring dashes
	 *
	 */
	void lookForInvalidOptionsDashInsens() {
		auto flagStrs = flags_.getFlags();
		strVecToLower(flagStrs);
		for(auto & f : flagStrs){
			lstrip(f, '-');
		}
		for (const auto &com : commands_.arguments_) {
			if (!contains(flagStrs, lstripRet(com.first, '-'))) {
				warnings_.emplace_back(
						bashCT::bold + bashCT::red + "Unrecognized option, " + com.first
								+ " not using" + bashCT::reset);
				failed_ = true;
			}
		}
	}

	/**@brief Print any warnings incurred during set up to out
	 * @param out The std::ostream out object to print to
	 *
	 */
	void printWarnings(std::ostream &out) const {
		for (const auto &warn : warnings_) {
			out << warn << std::endl;
		}
	}
	/**@brief Adding warning to the warning messages
	 *
	 * @param warn The message
	 */
	void addWarning(const std::string & warn) {
		warnings_.emplace_back(warn);
	}

	/**@brief Print flags but looking at validOptions_ so needs to be called at
	 * the end of set up so that options can be placed into validOptions_
	 * @param out The std::ostream out object to print to
	 */
	void printFlags(std::ostream &out) {

		std::map<std::string, std::map<std::string, std::string>> infoOutRequried;
		std::map<std::string, std::map<std::string, std::string>> infoOutNotRequried;
		for (const auto & f : flags_.flags_) {
			if (f.second.required_) {
				infoOutRequried[f.second.flagGrouping_][f.second.getFlagsStrAutoDash()] = f.second.helpInfo();
			} else {
				infoOutNotRequried[f.second.flagGrouping_][f.second.getFlagsStrAutoDash()] = f.second.helpInfo();
			}
		}
		std::map<std::string, std::string> infoOutHelp;
		infoOutHelp["--getFlags"] = "Print flags";
		infoOutHelp["-h,--help"] = "Print a more detail help message if available";
		std::pair<uint32_t, uint32_t> paddings { 0, 0 };
		auto keyFunc = [](const auto & kv) {return kv.first;};
		auto valFunc =
				[](const auto & kv) {return bashCT::trimForNonTerminalOut(kv.second);};
		if (!infoOutRequried.empty()) {
			for(const auto & requiredFlagGroup : infoOutRequried){
				compareColPaddings(requiredFlagGroup.second, paddings, keyFunc, valFunc);
			}
		}
		if (!infoOutNotRequried.empty()) {
			for(const auto & optionalFlagGroup : infoOutNotRequried){
				compareColPaddings(optionalFlagGroup.second, paddings, keyFunc, valFunc);
			}
		}
		if (!infoOutHelp.empty()) {
			compareColPaddings(infoOutHelp, paddings, keyFunc, valFunc);
		}
		const std::string middleSep = "\t";
		out << commands_.masterProgram_ << " " << commands_.subProgramRaw_ << std::endl;
		if("" != description_){
			out << description_ << std::endl;
		}
		//assuming tab is len of 4
		std::string requiredOptsTitle = bashCT::bold + bashCT::green + "Required Options"
				+ bashCT::reset;
		std::string optionalOptsTitle = bashCT::bold + bashCT::blue + "Optional Options"
				+ bashCT::reset;
		std::string helpOptsTitle = bashCT::bold + "Help Options" + bashCT::reset;
		if (!infoOutRequried.empty()) {
			out
					<< std::string(
							paddings.first + 2
									- round(bashCT::getPrintLen(requiredOptsTitle) / 2.0), ' ')
					<< requiredOptsTitle << std::endl;
			for (const auto & requiredFlagGroup : infoOutRequried) {
				if ("Misc" == requiredFlagGroup.first) {
					continue;
				}
				std::string currentTitle = bashCT::bold + bashCT::purple
						+ requiredFlagGroup.first + bashCT::reset;
				;
				out
						<< std::string(
								paddings.first + 2
										- round(bashCT::getPrintLen(currentTitle) / 2.0), ' ')
						<< currentTitle << std::endl;
				mapOutColAdjust(requiredFlagGroup.second, out, middleSep,
						paddings.first, paddings.second);
			}
			if(in(std::string("Misc"), infoOutRequried)){
				std::string miscTitle = bashCT::bold + bashCT::purple  + "Miscellaneous" + bashCT::reset;;
				out
						<< std::string(
								paddings.first + 2
										- round(bashCT::getPrintLen(miscTitle) / 2.0), ' ')
						<< miscTitle << std::endl;
				mapOutColAdjust(infoOutRequried.at("Misc"), out, middleSep, paddings.first,
						paddings.second);
			}
		}
		if (!infoOutNotRequried.empty()) {
			out
					<< std::string(
							paddings.first + 2
									- round(bashCT::getPrintLen(optionalOptsTitle) / 2.0), ' ')
					<< optionalOptsTitle << std::endl;
			for(const auto & optionalFlagGroup : infoOutNotRequried){
				if("Misc" == optionalFlagGroup.first){
					continue;
				}
				std::string currentTitle = bashCT::bold + bashCT::purple  + optionalFlagGroup.first + bashCT::reset;;
				out
						<< std::string(
								paddings.first + 2
										- round(bashCT::getPrintLen(currentTitle) / 2.0), ' ')
						<< currentTitle << std::endl;
				mapOutColAdjust(optionalFlagGroup.second, out, middleSep, paddings.first,
						paddings.second);
			}
			if(in(std::string("Misc"), infoOutNotRequried)){
				std::string miscTitle = bashCT::bold + bashCT::purple  + "Miscellaneous" + bashCT::reset;;
				out
						<< std::string(
								paddings.first + 2
										- round(bashCT::getPrintLen(miscTitle) / 2.0), ' ')
						<< miscTitle << std::endl;
				mapOutColAdjust(infoOutNotRequried.at("Misc"), out, middleSep, paddings.first,
						paddings.second);
			}
		}
		if (!infoOutHelp.empty()) {
			out
					<< std::string(
							paddings.first + 2
									- round(bashCT::getPrintLen(helpOptsTitle) / 2.0), ' ')
					<< helpOptsTitle << std::endl;
			mapOutColAdjust(infoOutHelp, out, middleSep, paddings.first,
					paddings.second);
		}
		if (!examples_.empty()) {
			out << std::string(
					paddings.first + 2
							- round(bashCT::getPrintLen(helpOptsTitle) / 2.0), ' ')
			<< bashCT::boldGreen("Examples") << std::endl;
			for (const auto & example : examples_) {
				out
						<< njh::replaceString(
								njh::replaceString(example, "MASTERPROGRAM",
										commands_.masterProgram_), "SUBPROGRAM",
								commands_.subProgramRaw_) << std::endl;
			}
		}
	}

	/**@brief Once all options have been looked for call finishSetUp() so that any
	 *warnings or help messages can be print and to find out if set up was
	 *successful
	 * @param out The std::ostream out object to print to
	 *
	 */
	void finishSetUp(std::ostream &out = std::cout) {
		if (commands_.printingHelp() || commands_.gettingFlags() ) {
			printFlags(out);
			exit(1);
		}
		lookForInvalidOptionsDashInsens();
		printWarnings(out);
		if (failed_) {
			exit(1);
		}
	}

	struct FlagCheckResult {
		FlagCheckResult(bool succes, const std::string & mes) :
				succes_(succes), message_(mes) {
		}
		bool succes_;
		std::string message_;
	};

	/**@brief A templated function to look for options and implementation for
	 *setting the option is handled by commandLineArguments so look there for more
	 *details
	 * @param option The option to set
	 * @param flag The flag to be searched for
	 * @param shortDescription The name under which to store the option being searched for
	 * @param required A bool indicating if the option is required and set up
	 *should be stopped if not found, default is false
	 *
	 * @return Returns true if option is found or false if option is not found
	 *
	 */
	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required = false) {
		return setOption(option, flagStr, shortDescription, required, "Misc");
	}



	/**@brief A templated function to look for options and implementation for
	 *setting the option is handled by commandLineArguments so look there for more
	 *details
	 * @param option The option to set
	 * @param flagStr The flag to be searched for
	 * @param shortDescription The name under which to store the option being searched for
	 * @param required A bool indicating if the option is required and set up should be stopped if not found, default is false
	 * @param flagGrouping A grouping for the flag
	 *
	 * @return Returns true if option is found or false if option is not found
	 *
	 */
	template<typename T>
	bool setOption(T &option, std::string flagStr,
			const std::string & shortDescription, bool required,
			const std::string & flagGrouping) {
		bool found = false;
		try {
			Flag currentFlag(option, flagStr, shortDescription, required, flagGrouping);
			std::vector<std::string> flagsFound;
			for (const auto &fTok : currentFlag.flags_) {
				if (commands_.lookForOptionDashCaseInsen(option, fTok)) {
					currentFlag.setValue(option);
					found = true;
					flagsFound.emplace_back(fTok);
				}
			}
			if (required && !found) {
				std::stringstream tempStream;
				tempStream << bashCT::bold + bashCT::black << "Need to have "
						<< bashCT::red << conToStr(tokenizeString(flagStr, ","), " or ")
						<< bashCT::black << " see " << bashCT::red
						<< commands_.getProgramName() + " --help " << bashCT::black
						<< "for more details" << bashCT::reset;
				warnings_.emplace_back(tempStream.str());
				failed_ = true;
			}
			if (found && flagsFound.size() > 1) {
				std::stringstream tempStream;
				tempStream << "Found multiple flags for the same option, found "
						<< conToStr(flagsFound, ", ") << " but should only have one";
				warnings_.emplace_back(tempStream.str());
				failed_ = true;
			}
			flags_.addFlag(currentFlag);
		} catch (std::exception & e) {
			std::stringstream ss;
			ss << "Error setting option for " << flagStr << "\n";
			ss << "Exception: " << e.what() << "\n";
			throw std::runtime_error{ss.str()};
		}
		return found;
	}

	/**@brief A templated function to look for options and implementation for
	 *setting the option is handled by commandLineArguments so look there for more
	 *details
	 * @param option The option to set
	 * @param flagStr The flag to be searched for
	 * @param shortDescription The name under which to store the option being searched for
	 * @param required A bool indicating if the option is required and set up should be stopped if not found, default is false
	 * @param flagGrouping A grouping for the flag
	 *
	 * @return Returns true if option is found or false if option is not found
	 *
	 */
	template<typename T>
	bool setOption(T &option, std::string flagStr,
			const std::string & shortDescription, bool required,
			const std::string & flagGrouping,
			const std::function<FlagCheckResult(const T&)> &  testFunc) {
		bool ret = setOption(option, flagStr, shortDescription, required, flagGrouping);
		auto testResult =  testFunc(option);
		if(!testResult.succes_){
			failed_ = true;
			addWarning(testResult.message_);
		}
		return ret;
	}

	/**@brief A templated function to look for options and implementation for
	 *setting the option is handled by commandLineArguments so look there for more
	 *details
	 * @param option The option to set
	 * @param flagStr The flag to be searched for
	 * @param shortDescription The name under which to store the option being searched for
	 * @param required A bool indicating if the option is required and set up
	 *should be stopped if not found, default is false
	 * @param testFunc A function to test the set option
	 *
	 * @return Returns true if option is found or false if option is not found
	 *
	 */
	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required,
			const std::function<FlagCheckResult(const T&)> &  testFunc) {
		return setOption(option, flagStr, shortDescription, required, "Misc", testFunc);
	}

	/**@brief A templated function to look for options and implementation for
	 *setting the option is handled by commandLineArguments so look there for more
	 *details
	 * @param option The option to set
	 * @param flagStr The flag to be searched for
	 * @param shortDescription The name under which to store the option being searched for
	 * @param testFunc A function to test the set option
	 *
	 * @return Returns true if option is found or false if option is not found
	 *
	 */
	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			const std::function<FlagCheckResult(const T&)> &  testFunc) {
		return setOption(option, flagStr, shortDescription, false, "Misc", testFunc);
	}

	enum class CheckCase{
		NONZERO,
		GREATERZERO,
		NONE
	};

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			const CheckCase &  testFuncCase) {
		return setOption(option, flagStr, shortDescription, false, testFuncCase);
	}

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required,
			const CheckCase &  testFuncCase) {
		return setOption(option, flagStr, shortDescription, required, "Misc", testFuncCase);
	}

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required,
			const std::string &flagGrouping,
			const CheckCase &  testFuncCase) {
		if(testFuncCase == CheckCase::NONZERO){
			return setOption(option, flagStr, shortDescription, required, flagGrouping, flagCheckNonZero<T>(flagStr));
		}else if(testFuncCase == CheckCase::GREATERZERO){
			return setOption(option, flagStr, shortDescription, required, flagGrouping, flagCheckGreaterThanZero<T>(flagStr));
		}else{
			return setOption(option, flagStr, shortDescription, required, flagGrouping);
		}
	}

	enum class ConCheckCase{
		NONZERO,
		GREATERZERO,
		NONE
	};

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			const ConCheckCase &  testFuncCase) {
		return setOption(option, flagStr, shortDescription, false, testFuncCase);
	}

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required,
			const ConCheckCase &  testFuncCase) {
		return setOption(option, flagStr, shortDescription, required, "Misc", testFuncCase);
	}

	template<typename T>
	bool setOption(T &option,
			std::string flagStr,
			const std::string &shortDescription,
			bool required,
			const std::string &flagGrouping,
			const ConCheckCase &  testFuncCase) {
		if(testFuncCase == ConCheckCase::NONZERO){
			return setOption(option, flagStr, shortDescription, required, flagGrouping, flagCheckNonZeroContainers<T>(flagStr));
		}else if(testFuncCase == ConCheckCase::GREATERZERO){
			return setOption(option, flagStr, shortDescription, required, flagGrouping, flagCheckGreaterThanZeroContainers<T>(flagStr));
		}else{
			return setOption(option, flagStr, shortDescription, required, flagGrouping);
		}
	}


	/**@brief Generator for a flag check for a value that cannot be zero
	 *
	 * @param flagName
	 * @return a function that tests a value for being less than or equal to zero
	 */
	template<typename T>
	static std::function<FlagCheckResult(const T&)> flagCheckNonZero(
			const std::string & flagName) {
		std::function<FlagCheckResult(const T&)> ret =
				[&flagName](const T & val) {
					if(0 == val) {
						return njh::progutils::ProgramSetUp::FlagCheckResult(false, flagName + " can't be zero");
					}
					return njh::progutils::ProgramSetUp::FlagCheckResult(true, "");
				};
		return ret;
	}

	/**@brief Generator for a flag check that must be greater than zero
	 *
	 * @param flagName
	 * @return a function that tests a value for being less than or equal to zero
	 */
	template<typename T>
	static std::function<FlagCheckResult(const T&)> flagCheckGreaterThanZero(
			const std::string & flagName) {
		std::function<FlagCheckResult(const T&)> ret =
				[&flagName](const T & val) {
					if(val <= 0 ) {
						return njh::progutils::ProgramSetUp::FlagCheckResult(false, flagName + " can't be zero or less: " + estd::to_string(val));
					}
					return njh::progutils::ProgramSetUp::FlagCheckResult(true, "");
				};
		return ret;
	}


	template<typename T>
	static std::function<FlagCheckResult(const T&)> flagCheckNonZeroContainers(
			const std::string & flagName) {
		std::function<FlagCheckResult(const T&)> ret =
				[&flagName](const T & con) {
					if(std::any_of(con.begin(), con.end(), [](const typename T::value_type & val){
						return 0 == val;
					})) {
						return njh::progutils::ProgramSetUp::FlagCheckResult(false, flagName + " can't contain any zero values: " + njh::conToStr(con, ","));
					}
					return njh::progutils::ProgramSetUp::FlagCheckResult(true, "");
				};
		return ret;
	}

	template<typename T>
	static std::function<FlagCheckResult(const T&)> flagCheckGreaterThanZeroContainers(
			const std::string & flagName) {
		std::function<FlagCheckResult(const T&)> ret =
				[&flagName](const T & con) {
					if(std::any_of(con.begin(), con.end(), [](const typename T::value_type & val){
						return val <= 0;
					})) {
						return njh::progutils::ProgramSetUp::FlagCheckResult(false, flagName + " can't contain any zero values: " + njh::conToStr(con, ","));
					}
					return njh::progutils::ProgramSetUp::FlagCheckResult(true, "");
				};
		return ret;
	}


	/**@brief Get the current run time since time point start_ in seconds
	 * @return A double with the run time converted into seconds
	 *
	 */
	double getRunTimeInSecs() {
		return timer_.totalTime();
	}

	/**@brief Get a string formated run time in seconds rounded to the nearest
	 * hundreth
	 * @return A string wiht run time formated to tell time for hrs, minutes,
	 * seconds etc.
	 */
	std::string getRunTime() {
		return commands_.getProgramName() + " (" + timer_.totalTimeFormatted(2) + ")";
	}

	/**@brief Function to log the run time to a std::ostream
	 * @param out The std::ostream to log the info to
	 */
	void logRunTime(std::ostream &out) {
		out << getRunTime() << std::endl;
	}

	/**@brief A function to find out if help is need or required, called with a
	 * minimum arguments parameter to indicated the num of args to print out if no
	 * help flag is indicated
	 * @param minAmountOfArgs Number of arguments to print help, defualts to one
	 * so if only the program name is given it indicates help is needed
	 * @return return true if a help message should be printed or false if no help
	 * is needed
	 */
	bool needsHelp(uint32_t minAmountOfArgs = 0) {
		return (commands_.hasFlagCaseInsen("--help")
				|| commands_.hasFlagCaseInsen("-h")
				|| commands_.numberOfCommands() <= minAmountOfArgs);
	}

	/**@brief A Function to print out all the current arguments in an organized
	 *table
	 * @param commandLineArgumentsMap A map of strings pairs of the current
	 *arguments
	 * @param out The std::ostream object to print the info to
	 *
	 */
	void writeOutCommandLineArguments(
			const std::map<std::string, std::string> &commandLineArgumentsMap,
			std::ostream &out) {
		uint32_t optionCount = 0;
		std::vector<std::string> columnNames = { "OptionNum", "Flag", "Option" };
		std::vector<std::vector<std::string>> content;
		for (const auto &mContent : commandLineArgumentsMap) {
			content.emplace_back(
					std::vector<std::string> { estd::to_string(optionCount) + ")",
							mContent.first, mContent.second });
			++optionCount;
		}
		printTableOrganized(content, columnNames, out);
	}
};

}  // namespace progutils
}  // namespace njh
