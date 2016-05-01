#pragma once
//
//  programRunner.hpp
//
//
//  Created by Nicholas Hathaway on 9/27/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibcpp/utils.h"
#include "bibcpp/progutils/programSetUp.hpp"
#include "bibcpp/concurrency.h"
#include <thread>
namespace bib{
namespace progutils {

/**@brief programRunner class for running several subprograms stored in a map of
 *function objects
 *
 */
class programRunner {
 protected:
  /**@brief funcInfo struct that holds
   *
   */
  struct funcInfo {
    /**@brief A std::func object holding the function call of the sub program
     *
     */
    std::function<int(const CmdArgs &)> func_;
    /**@brief The title of the stored program
     *
     */
    std::string title_;
    /**@brief whether the program title is an alias and shouldn't be printed
     *
     */
    bool alias_;
  };
  /**@brief A map of key being function name and a funcInfo struct, see funcInfo
   *for more info
   *
   * This needs to be initialized with the desired subprograms in order for them
   *to be able to be called
   *
   */
  const std::map<std::string, funcInfo> cmdToFunc_;

 public:
  /**@brief Name of master program that holds the sub-programs
   *
   */
  const std::string nameOfProgram_;
  /**@brief Constructor with map of funcInfo structs holding the sub programs and the
   *name of the master program
   *
   */
  programRunner(std::map<std::string, funcInfo> cmdToFunc,
                std::string nameOfProgram)
      : cmdToFunc_(cmdToFunc), nameOfProgram_(nameOfProgram) {}

  virtual ~programRunner() {}
  /**@brief Run one of the subprogram with given arguments stored in argv if
   *master program contains it
   *
   * Checks to see if a batch of commands is desired
   *
   * @param argc The number of arguments
   * @param argv The array of char pointers of the arguments
   * @return A int indicating success of run
   *
   */
  virtual int run(int argc, char *argv[]) {
    if (argc < 2) {
      listPrograms(std::cout,"", nameOfProgram_);
      return 0;
    }
    auto cmds = CmdArgs(argc, argv);
    if ("batch" == cmds.subProgramLowerCase()) {
      return batchRun(cmds);
    }else if ("batchthreaded" == cmds.subProgramLowerCase()) {
      return batchRunThreaded(cmds);
    }
    return runProgram(cmds);
  }

  /** @brief Give a map of string string pairs of arguments for the program
   *running where first member is flag and second member is argument
   *
   * @param inputCommands A map of string pairs, key is flag and value is flag
   *associated value
   * @return An int indicating success of running the program
   */
  virtual int runProgram(const CmdArgs & args) const {
    if (containsProgram(args.subProgramLowerCase())) {
      const auto &fi = cmdToFunc_.at(args.subProgramLowerCase());
      return fi.func_(args);
    }
    listPrograms(std::cout, args.subProgram_, nameOfProgram_);
    return 1;
  }

  /**@brief List the contained subprograms and if given with a command option,
   *print the closest sub-program
   *
   * @param out The std::ostream object to print the info to
   * @param command If this is not blank it indicates that an unrecognized
   *command was found making listPrograms() print the closest subprogram
   * @param nameOfProgram If this does not equal the name of current program it
   *means another program called it
   *
   */
 public:
	virtual void listPrograms(std::ostream &out, const std::string &command,
			const std::string &nameOfProgram) const {
		if (nameOfProgram == nameOfProgram_) {
			out << "Programs" << std::endl;
			out << "Use " << nameOfProgram_
					<< " [PROGRAM] --help to see more details about each program"
					<< std::endl;
			out << "Commands are not case sensitive" << std::endl;
			listCommands(out);
			if (command != "") {
				auto closet = closestProgram(command);
				out << "Unrecognized command " << command << std::endl;
				out << "Did you mean         " << closet.first << "?" << std::endl;
			}
		} else {
			//this if for when the program runner is a sub ring in a one ring
			out << nameOfProgram_ << std::endl;
			listCommands(out);
		}
	}
  /**@brief A function to check to see if current program contains the
   *sub-program
   *
   * @param program Name of subprogram to search for
   */
  virtual bool containsProgram(const std::string &program) const {
    return cmdToFunc_.find(program) != cmdToFunc_.end();
  }
	/** @brief A function to find closest matching sub-program, done by giving the
	 *program with the highest global alignment score to the input parameter
	 *program
	 *
	 * @param program The program to compare to the sub-programs
	 */
	virtual std::pair<std::string, int> closestProgram(
			const std::string &program) const {
		std::pair<std::string, int> ret = { "", 0 };
		if ("" == program) {
			return ret;
		}
		for (const auto &prog : cmdToFunc_) {
			int32_t currentScore = needleScore(program, prog.first);
			if (currentScore > ret.second) {
				ret = {prog.second.title_, currentScore};
			}
		}
		return ret;
	}
  /**@brief A function to run a subprogram with same parameters but on all files
   *with a certain file extension
   *
   * @param inputCommands A map of string pairs with argument and flag pairs,
   *should contain -ending for file extension and -run for subprogram to run
   * @return int indicating run success
   *
   */
  virtual int batchRun(CmdArgs inputCommands) {
    std::vector<std::string> batchFlags{"-ending", "-pattern", "-run", "batch"};
    std::string ending = "", program = "", pattern = "";
    programSetUp setUp(inputCommands);
    bool endFlag = setUp.setOption(ending , "-ending", "A file extension to run batch commands on", false);
    setUp.setOption(pattern, "-pattern", "File Pattern to run batch command", !endFlag);
    setUp.setOption(program, "-run", "ProgramToRun", true);
    if(setUp.gettingFlags_){
    	std::cout << bashCT::boldGreen("Batch") << bashCT::boldBlack(" Commands") << std::endl;
    	setUp.printFlags(std::cout);
    	if(program != ""){
    		inputCommands.subProgram_ = program;
        for(const auto & flag : batchFlags){
        	inputCommands.removeArgumentCaseInsen(flag);
        }
      	std::cout << bashCT::boldGreen(program) << bashCT::boldBlack(" Commands") << std::endl;
        runProgram(inputCommands);
    	}
    	exit(1);
    }
    setUp.printWarnings(std::cout);
    if (setUp.failed_) {
      exit(1);
    }

    // erase the batchRun flags to remove
    bool overWrite = false;
    bool exitOnFailureToWrite = true;
    // create run log
    std::ofstream runLog;
    std::string nameOfRunLog =
        "batchRunLog_" + inputCommands.masterProgram_ + "-" + program + "_" + getCurrentDate() + ".txt";
    files::openTextFile(runLog, nameOfRunLog, ".txt", overWrite, exitOnFailureToWrite);
    runLog << "Ran " << getCurrentDate() << std::endl;
    runLog << "Ran from: " << inputCommands.workingDir_ << std::endl;
    runLog << inputCommands.commandLine_ << std::endl;
    for(const auto & flag : batchFlags){
    	inputCommands.removeArgumentCaseInsen(flag);
    }
    inputCommands.subProgram_ = program;
    // gather the necessary files
    if(endFlag){
    	pattern = ".*" + ending;
    }
    std::vector<files::bfs::path> specificFiles;
  	auto allFiles = bib::files::filesInFolder(".");
  	for(const auto & f : allFiles){
  		if(std::regex_match(f.filename().string(),std::regex{pattern})){
  			specificFiles.emplace_back(f);
  		}
  	}
    // run the command on each file
    for (const auto &file : specificFiles) {
      if (containsSubString(file.string(), "batchRunLog")) {
        continue;
      }
      CmdArgs currentCommands = inputCommands;
      for (auto &com : currentCommands.arguments_) {
        com.second = replaceString(com.second, "THIS", file.filename().string() );
      }
      currentCommands.resetCommandLine();
      // log current run command
      std::cout << currentCommands.commandLine_ << std::endl;
      runLog << currentCommands.commandLine_ << std::endl;
      std::cout << std::endl;
      // run the current command
      stopWatch watch;
      runProgram(currentCommands);
      std::cout << "\tCurrent Command Run Time: " << watch.totalTimeFormatted(6) << std::endl;
      runLog << "\tCurrent Command Run Time: " << watch.totalTimeFormatted(6) << std::endl;
      setUp.logRunTime(runLog);
      setUp.logRunTime(std::cout);
      std::cout << std::endl;
    }
    setUp.logRunTime(runLog);
    setUp.logRunTime(std::cout);
    return 0;
  }
  /**@brief A function to run a subprogram with same parameters but on all files
   *with a certain file extension
   *
   * @param inputCommands A map of string pairs with argument and flag pairs,
   *should contain -ending for file extension and -run for subprogram to run and
   *-numThreads for number of threads
   * @return int indicating run success
   *
   */
  virtual int batchRunThreaded(CmdArgs inputCommands) {
    std::vector<std::string> batchFlags{"-ending", "-pattern", "-run", "batchthreaded", "-batchthreads"};
    std::string ending = "", program = "", pattern = "";
    programSetUp setUp(inputCommands);
    bool endFlag = setUp.setOption(ending , "-ending", "A file extension to run batch commands on", false);
    setUp.setOption(pattern, "-pattern", "File Pattern to run batch command", !endFlag);
    setUp.setOption(program, "-run", "Program To Run a Batch of Commands with", true);
    uint32_t numThreads = 2;
    setUp.setOption(numThreads, "-batchThreads", "Number of Threads use for the batch commands");
    if(setUp.gettingFlags_){
    	std::cout << bashCT::boldGreen("Batch") << bashCT::boldBlack(" Commands") << std::endl;
    	setUp.printFlags(std::cout);
    	if(program != ""){
    		inputCommands.subProgram_ = program;
        for(const auto & flag : batchFlags){
        	inputCommands.removeArgumentCaseInsen(flag);
        }
      	std::cout << bashCT::boldGreen(program) << bashCT::boldBlack(" Commands") << std::endl;
        runProgram(inputCommands);
    	}
    	exit(1);
    }
    setUp.printWarnings(std::cout);
    if (setUp.failed_) {
      exit(1);
    }

    // erase the batchRun flags to remove
    bool overWrite = false;
    bool exitOnFailureToWrite = true;
    // create run log
    std::ofstream runLog;
    std::string nameOfRunLog =
        "batchRunLog_" + inputCommands.masterProgram_ + "-" + program + "_" + getCurrentDate() + ".txt";
    files::openTextFile(runLog, nameOfRunLog, ".txt", overWrite, exitOnFailureToWrite);
    runLog << "Ran " << getCurrentDate() << std::endl;
    runLog << "Ran from: " << inputCommands.workingDir_ << std::endl;
    runLog << inputCommands.commandLine_ << std::endl;
    for(const auto & flag : batchFlags){
    	inputCommands.removeArgumentCaseInsen(flag);
    }
    inputCommands.subProgram_ = program;
    // gather the necessary files
    if(endFlag){
    	pattern = ".*" + ending;
    }
    std::vector<files::bfs::path> specificFiles;
  	auto allFiles = bib::files::filesInFolder(".");
  	for(const auto & f : allFiles){
  		if(std::regex_match(f.filename().string(),std::regex{pattern})){
  			specificFiles.emplace_back(f);
  		}
  	}
    // run the command on each file
    std::vector<std::shared_ptr<CmdArgs>> allCommands;
    for (const auto &file : specificFiles) {
      if (containsSubString(file.string(), "batchRunLog")) {
        continue;
      }
      CmdArgs currentCommands = inputCommands;
      for (auto &com : currentCommands.arguments_) {
        com.second = replaceString(com.second, "THIS", file.filename().string() );
      }
      currentCommands.resetCommandLine();
      allCommands.emplace_back(std::make_shared<CmdArgs>(currentCommands));
    }
    concurrent::LockableQueue<std::shared_ptr<CmdArgs>> argPool(allCommands);
  	std::vector<std::thread> threads;
  	std::mutex logMut;
  	auto runCmds = [this,&logMut,&runLog](concurrent::LockableQueue<std::shared_ptr<CmdArgs>> & pool){
  				std::shared_ptr<CmdArgs> currentCmd;
					while(pool.getVal(currentCmd)) {
			      stopWatch watch;
						runProgram(*currentCmd);
						{
							std::lock_guard<std::mutex> lock(logMut);
							runLog << currentCmd->commandLine_ << std::endl;
							runLog << "\tRun Time: " << watch.totalTimeFormatted(6) << std::endl;
						}
					}
  	};
  	for (uint32_t t = 0; t < numThreads; ++t) {
  		threads.emplace_back(std::thread(runCmds, std::ref(argPool)));
  	}
  	for (auto &t : threads) {
  		t.join();
  	}
    setUp.logRunTime(runLog);
    setUp.logRunTime(std::cout);
    return 0;
  }


 protected:
  /**@brief A function to add the subprogram funcInfo struct
   *
   * @param title The name of the subprogram
   * @param func The funcInfo struct containing the subprogram info, see
   *funcInfo for more info
   * @param alias A bool indicating if the title is an alias and shouldn't be
   *printed, can be used to hide subprograms
   * @return A pair of string, funcInfo for tile of subprogram and a funcInfo
   *struct wiht subprogram info
   *
   */
  template <typename T>
  static std::pair<std::string, funcInfo> addFunc(std::string title, T &func,
                                           bool alias = false) {
    auto name = strToLowerRet(title);
    return {name, {std::bind(&func, std::placeholders::_1), title, alias}};
  }
  /**@brief A function to list the subprograms, skipping ones marked with alias
   *
   * @param out The std::ostream object to print the info to
   */
  virtual void listCommands(std::ostream &out) const {
    size_t count = 0;
    for (const auto &e : cmdToFunc_) {
      if (!e.second.alias_) {
        ++count;
        out << leftPadNumStr(count, cmdToFunc_.size()) << ") "
            << e.second.title_ << std::endl;
      }
    }
  }
};

}  // namespace progutils
}  // namespace bib
