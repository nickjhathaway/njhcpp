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
    std::function<int(std::map<std::string, std::string>)> func_;
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
  /** Name of master program that holds the sub-programs
   *
   */
  const std::string nameOfProgram_;
  /**Constructor with map of funcInfo structs holding the sub programs and the
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
      listPrograms(std::cout);
      return 0;
    }
    std::stringstream commandLine;
    commandLineArguments::logRunArgumnets(commandLine, argc, argv);
    std::map<std::string, std::string> commands =
        commandLineArguments::convertCommandLineArguments(argc, argv);
    commands["-program"] = strToLowerRet(argv[1]);
    commands["-commandline"] = commandLine.str();

    if (commands["-program"] == "massrunwithending" ||
        commands["-program"] == "batch") {
      return massRunWithEnding(commands);
    }
    if (commands["-program"] == "massrunwithendingthreaded" ||
        commands["-program"] == "batchthreaded") {
      return massRunWithEndingThreaded(commands);
    }
    if (strAllDigits(commands["-program"])) {
      return runByNumber(commands["-program"], commands);
    }
    return runProgram(commands);
  }
  /** @brief Give a map of string string pairs of arguments for the program
   *runing
   *
   * @param inputCommands A map of string pairs, key is flag and value is flag
   *associated value
   * @return An int indicating succes of running the program
   */
  virtual int runProgram(std::map<std::string, std::string> inputCommands) const {
    if (containsProgram(inputCommands["-program"])) {
      const auto &fi = cmdToFunc_.at(inputCommands["-program"]);
      return fi.func_(inputCommands);
    }
    listPrograms(std::cout, inputCommands["-program"]);
    return 1;
  }
  /**@brief Give a vector of commands to run several programs in a row
   *
   * @param inputCommands a vector of maps of string pairs giving arguments
   * @return int indicating success of all programs
   */
  virtual int runProgram(std::vector<std::map<std::string, std::string>> inputCommands) const {
    int status = 0;
    for (const auto &com : inputCommands) {
      status = runProgram(com);
    }
    return status;
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
  virtual void listPrograms(std::ostream &out, const std::string &command = "",
                            const std::string &nameOfProgram = "programRunner")
      const {
    if (nameOfProgram == nameOfProgram_) {
      out << "Programs" << std::endl;
      out << "Use " << nameOfProgram_
          << " [PROGRAM] -help to see more details about each program"
          << std::endl;
      out << "Commands are not case sensitive" << std::endl;
      listCommands(out);
      if (command != "") {
        auto closet = closestProgram(command);
        out << "Unrecognized command " << command << std::endl;
        out << "Did you mean         " << closet.first << "?" << std::endl;
      }
    } else {
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
  virtual std::pair<std::string, int> closestProgram(const std::string &program)
      const {
    std::pair<std::string, int> ans = {"", 0};
    for (const auto &prog : cmdToFunc_) {
      int32_t currentScore = needleScore(program, prog.first);
      if (currentScore > ans.second) {
        ans = {prog.first, currentScore};
      }
    }
    return ans;
  }
  /**@brief A function to run a subprogram with same parameters but on all files
   *with a certain file extention
   *
   * @param inputCommands A map of string pairs with argument and flag pairs,
   *should contain -ending for file extention and -run for subprogram to run
   * @return int indicating run success
   *
   */
  virtual int massRunWithEnding(std::map<std::string, std::string> inputCommands) {
    std::string ending = "", program = "";
    programSetUp setUp(inputCommands);
    setUp.setOption(ending, "-ending", "FileEnding", true);
    setUp.setOption(program, "-run", "ProgramToRun", true);
    setUp.printWarnings(std::cout);
    if (setUp.failed_) {
      exit(1);
    }
    bool overWrite = false;
    bool exitOnFailureToWrite = true;
    // create run log
    std::ofstream runLog;
    std::string nameOfRunLog =
        "massRunLog_" + inputCommands["-run"] + "_" + getCurrentDate() + ".txt";
    files::openTextFile(runLog, nameOfRunLog, ".txt", overWrite, exitOnFailureToWrite);
    runLog << "Ran " << getCurrentDate() << std::endl;
    runLog << inputCommands.find("-commandline")->second << std::endl;
    // erase the massRunWithEnding flags to remove
    inputCommands.erase("-ending");
    inputCommands.erase("-program");
    inputCommands.erase("massrunwithending");
    inputCommands.erase("batch");
    // gather the neccessary files
    auto allFiles = files::listAllFiles(".", false, std::vector<std::string>{});
    std::vector<files::bfs::path> specificFiles;
    for (const auto &file : allFiles) {
      if (file.first.string().size() > ending.size()) {
        if (file.first.string().substr(file.first.string().size() - ending.size(),
                              ending.size()) == ending) {
          specificFiles.emplace_back(file.first);
        }
      }
    }
    // run the command on each file
    for (const auto &file : specificFiles) {
      if (containsSubString(file.string(), "massRunLog")) {
        continue;
      }
      std::map<std::string, std::string> currentCommands = inputCommands;
      for (auto &com : currentCommands) {
        com.second = replaceString(com.second, "THIS", file.string());
      }
      // rebuild the commandline for each new run
      std::vector<std::string> toks = tokenizeString(currentCommands["-commandline"], "\n");
      std::string from = toks[0];
      std::stringstream currentCommandLine;
      currentCommandLine << from << std::endl;
      currentCommandLine << nameOfProgram_ <<  " " << currentCommands["-run"];
      currentCommands.erase("-run");
      for (auto &com : currentCommands) {
        if (com.first != "-commandline") {
          currentCommandLine << " " << com.first << " " << com.second;
        }
      }
      currentCommandLine << std::endl;
      // and now that the commandLine has been rebuilt for this run replace the
      // other one.
      currentCommands["-commandline"] = currentCommandLine.str();
      // set the program to be run
      currentCommands["-program"] = strToLowerRet(program);
      // log current run command
      std::cout << currentCommands["-commandline"] << std::endl;
      runLog << currentCommands["-commandline"] << std::endl;
      setUp.logRunTime(runLog);
      setUp.logRunTime(std::cout);
      std::cout << std::endl;
      // run the current command
      // std::cout<<currentCommands["-program"]<<std::endl;
      runProgram(currentCommands);
      std::cout << std::endl;
    }
    setUp.logRunTime(runLog);
    setUp.logRunTime(std::cout);
    return 0;
  }
  /**@brief A function to run a subprogram with same parameters but on all files
   *with a certain file extention
   *
   * @param inputCommands A map of string pairs with argument and flag pairs,
   *should contain -ending for file extention and -run for subprogram to run and
   *-numThreads for number of threads
   * @return int indicating run success
   *
   */
  virtual int massRunWithEndingThreaded(std::map<std::string, std::string> inputCommands) {
    std::string ending = "", program = "";
    programSetUp setUp(inputCommands);
    setUp.setOption(ending, "-ending", "FileEnding", true);
    setUp.setOption(program, "-run", "ProgramToRun", true);
    uint32_t numThreads = 2;
    setUp.setOption(numThreads, "-numThreads,-threads", "numThreads");
    setUp.printWarnings(std::cout);
    if (setUp.failed_) {
      exit(1);
    }
    bool overWrite = false;
    bool exitOnFailureToWrite = true;
    // create run log
    std::ofstream runLog;
    std::string nameOfRunLog =
        "massRunLog_" + inputCommands["-run"] + "_" + getCurrentDate() + ".txt";
    files::openTextFile(runLog, nameOfRunLog, ".txt", overWrite, exitOnFailureToWrite);
    runLog << "Ran " << getCurrentDate() << std::endl;
    runLog << inputCommands.find("-commandline")->second << std::endl;
    // erase the massRunWithEnding flags to remove
    inputCommands.erase("-ending");
    inputCommands.erase("-program");
    inputCommands.erase("massrunwithendingthreaded");
    inputCommands.erase("batchthreaded");
    if (inputCommands.find("-numThreads") != inputCommands.end()) {
      inputCommands.erase("-numThreads");
    }
    if (inputCommands.find("-numthreads") != inputCommands.end()) {
      inputCommands.erase("-numthreads");
    }
    if (inputCommands.find("-threads") != inputCommands.end()) {
      inputCommands.erase("-threads");
    }
    // gather the necessary files
    auto allFiles = files::listAllFiles(".", false, std::vector<std::string>{});
    std::vector<files::bfs::path> specificFiles;
    for (const auto &file : allFiles) {
      if (file.first.string().size() > ending.size()) {
        if (file.first.string().substr(file.first.string().size() - ending.size(),
                              ending.size()) == ending) {
          specificFiles.emplace_back(file.first);
        }
      }
    }
    // run the command on each file
    std::vector<std::map<std::string, std::string>> allCommands;
    for (const auto &file : specificFiles) {
      if (containsSubString(file.string(), "massRunLog")) {
        continue;
      }
      std::map<std::string, std::string> currentCommands = inputCommands;
      for (auto &com : currentCommands) {
        com.second = replaceString(com.second, "THIS", file.string());
      }

      // rebuild the commandline for each new run
      std::vector<std::string> toks = tokenizeString(currentCommands["-commandline"], "\n");
      std::string from = toks[0];
      std::stringstream currentCommandLine;
      currentCommandLine << from << std::endl;
      currentCommandLine << nameOfProgram_ << " " << currentCommands["-run"];
      currentCommands.erase("-run");
      for (auto &com : currentCommands) {
        if (com.first != "-commandline") {
          currentCommandLine << " " << com.first << " " << com.second;
        }
      }
      currentCommandLine << std::endl;
      // and now that the commandLine has been rebuilt for this run replace the
      // other one.
      currentCommands["-commandline"] = currentCommandLine.str();
      // set the program to be run
      currentCommands["-program"] = strToLowerRet(program);
      // log current run command
      std::cout << currentCommands["-commandline"] << std::endl;
      // runLog << currentCommands["-commandline"] << std::endl;
      // setUp.logRunTime(runLog);
      setUp.logRunTime(std::cout);
      std::cout << std::endl;
      // run the current command
      // std::cout<<currentCommands["-program"]<<std::endl;
      // runProgram(currentCommands);
      allCommands.emplace_back(currentCommands);
      std::cout << std::endl;
    }
    std::vector<std::vector<std::map<std::string, std::string>>> splitCommands(numThreads);
    uint32_t comPos = 0;
    while (comPos < allCommands.size()) {
      uint32_t tNum = 0;
      while (tNum < numThreads && comPos < allCommands.size()) {
        splitCommands[tNum].emplace_back(allCommands[comPos]);
        ++tNum;
        ++comPos;
      }
    }
    std::vector<std::thread> threads;
    for (uint32_t t = 0; t < numThreads; ++t) {
      threads.emplace_back(
          std::thread([&](std::vector<std::map<std::string, std::string>> coms) { runProgram(coms); },
                      splitCommands[t]));
    }
    for (auto &t : threads) {
      t.join();
    }
    setUp.logRunTime(runLog);
    setUp.logRunTime(std::cout);
    return 0;
  }

  int runByNumber(std::string numStr, std::map<std::string, std::string> inputCommands) const {
    if (strAllDigits(numStr)) {
      return runByNumber(std::stoi(numStr), inputCommands);
    }
    std::cout << "NumStr isn't a number: " << numStr << std::endl;
    return 1;
  }
  int runByNumber(size_t num, std::map<std::string, std::string> inputCommands) const {
    size_t count = 0;
    for (const auto &e : cmdToFunc_) {
      if (!e.second.alias_) {
        ++count;
        if (count == num) {
          const auto &fi = e.second;
          inputCommands["-program"] = fi.title_;
          return fi.func_(inputCommands);
        }
      }
    }
    std::cout << "Num: " << num
              << " is greater than size of command list: " << cmdToFunc_.size()
              << std::endl;
    return 1;
  }

 protected:
  /**@brief A function to add the subprogram funcInfo struct
   *
   * @param title The name of the subprogram
   * @param func The funcInfo struct containing the subprogram info, see
   *funcInfo for more info
   * @param alias A bool indicating if the title is an alias and shouldn't be
   *printed, can be used to hide subprograms
   * @param lower A bool whether the number should be changed to lower case for
   *the check, default is true
   * @return A pair of string, funcInfo for tile of subprogram and a funcInfo
   *struct wiht subprogram info
   *
   */
  template <typename T>
  static std::pair<std::string, funcInfo> addFunc(std::string title, T &func,
                                           bool alias, bool lower = true) {
    auto name = title;
    if (lower) {
      strToLower(name);
    }
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
