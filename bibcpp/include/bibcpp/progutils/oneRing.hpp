#pragma once
//
//  oneRing.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 11/22/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibcpp/progutils/programRunner.hpp"
namespace bib {
namespace progutils {
/**@brief A master runner to rule all sub-runners to package all sub-runners
 *into one place
 *
 * Inherits from programRunner which is what it stores as it's sub-runners, see
 *programRunner for more details about batching and other various class aspects
 *
 *
 */
class oneRing : public programRunner {

  /**@brief A map of key sub of sub-runner and value sub-runner
   *
   */
  const std::map<std::string, programRunner> rings_;

 protected:
  /**@brief Function to added a sub-runner, used in construtor
   *
   * @param runner A programRunner to bring under the rule of the current
   *oneRing program
   * @param lower A bool of whether or not to store the sub-runner's title as
   *lower case making look up case-insenstive
   * @return Returns a pair of string , programRunner to added ot the rings_
   *class member
   *
   *
   */
  std::pair<std::string, programRunner> addRing(programRunner runner,
                                                bool lower = true) {
    auto name = runner.nameOfProgram_;
    if (lower) {
      strToLower(name);
    }
    return {name, runner};
  }

  void listCommands(std::ostream &out) {}

 public:
  /**@brief Constructor with a map of sub-runners and any sub-programs of
   *current oneRing master runner
   *
   */
  oneRing(std::map<std::string, programRunner> rings,
          std::map<std::string, funcInfo> cmdToFunc, std::string nameOfProgram)
      : programRunner(cmdToFunc, nameOfProgram), rings_(rings) {}

  virtual ~oneRing() {};
  /**@brief Run the program by searching sub-runners and all sub-programs
   *defined in current oneRing program
   *
   * @param inputCommands A map of string pairs of arguments to run a sub-runner
   *
   */
  virtual int runProgram(std::map<std::string, std::string> inputCommands) const {
    std::string prog = inputCommands["-program"];
    if (containsProgram(prog)) {
      const auto &fi = cmdToFunc_.at(prog);
      return fi.func_(inputCommands);
    }

    // If given as PROGRAM_NUM will run the PROGRAM's NUM command
    std::string progName = "";
    std::string progNumber = "";
    if (prog.find("_") != std::string::npos) {
      std::vector<std::string> toks = tokenizeString(prog, "_");
      if (toks.size() == 2) {
        progName = toks[0];
        progNumber = toks[1];
        if (strAllDigits(progNumber)) {
          // inputCommands.erase(prog);
          if (rings_.find(progName) != rings_.end()) {
            return rings_.at(progName).runByNumber(progNumber, inputCommands);
          }
        }
      }
    }
    // if program is the number of one of the rings, list the program for that
    // ring
    if (rings_.find(prog) != rings_.end()) {
      rings_.at(prog).listPrograms(std::cout, "", "none");
      return 1;
    }
    // now try to find program in one of the rings
    for (auto &ring : rings_) {
      if (ring.second.containsProgram(prog)) {
        return ring.second.runProgram(inputCommands);
      }
    }
    listPrograms(std::cout, inputCommands["-program"]);
    return 1;
  }
  /**@brief List the programs listed in all sub-runner
   *
   * @param out The std::ostream object to print the info to
   * @param command A command to compare to all stored commands to find the
   *closest one
   * @param nameOfProgram The name of the current program, if it doesn't equal
   *the name of current program it is assumed another program is calling this
   *function
   *
   */
  virtual void listPrograms(std::ostream &out, const std::string &command = "",
                            const std::string &nameOfProgram = "oneRing")
      const {
    if (command != "") {
      out << "Unrecognized command " << command << std::endl;
    }
    out << "Programs" << std::endl;
    out << "Use " << nameOfProgram_
        << " [PROGRAM] -help to see more details about each program"
        << std::endl;
    out << "Commands are not case sensitive" << std::endl;

    for (auto &ring : rings_) {
      ring.second.listPrograms(std::cout, "", nameOfProgram);
    }
    if (command != "") {
      out << "Unrecognized command " << command << std::endl;
      std::pair<std::string, int> closestProgram = {"", 0};
      for (auto &ring : rings_) {
        std::pair<std::string, int> currentClosestProgram =
            ring.second.closestProgram(command);
        if (currentClosestProgram.second > closestProgram.second) {
          closestProgram = currentClosestProgram;
        }
      }
      std::cout << "Input command " << command << std::endl;
      std::cout << "Did you mean  " << closestProgram.first << "?" << std::endl;
    }
  }
};

}  // namespace progutils

}  // namespace bib
