#pragma once
//
//  oneRing.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 11/22/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibcpp/progutils/programRunner.hpp"
#include <memory>

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
  const std::map<std::string, std::shared_ptr<programRunner>> rings_;

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
	static std::pair<std::string, std::shared_ptr<programRunner>> addRing(
			const std::shared_ptr<programRunner> & runner) {
		auto name = strToLowerRet(runner->nameOfProgram_);
		return {name, runner};
	}
	template<typename T>
	static std::pair<std::string, std::shared_ptr<programRunner>> addRing() {
		auto runner = std::make_shared<T>();
		return addRing(runner);
	}

 public:
  /**@brief Constructor with a map of sub-runners and any sub-programs of
   *current oneRing master runner
   *
   */
  oneRing(std::map<std::string, std::shared_ptr<programRunner>> rings,
          std::map<std::string, funcInfo> cmdToFunc, std::string nameOfProgram)
      : programRunner(cmdToFunc, nameOfProgram), rings_(rings) {}

  virtual ~oneRing() {};
  /**@brief Run the program by searching sub-runners and all sub-programs
   *defined in current oneRing program
   *
   * @param inputCommands A map of string pairs of arguments to run a sub-runner
   *
   */
  virtual int runProgram(const CmdArgs & args) const {
  	//if the program is one of the programs in the oneRing then run that
    if (cmdToFunc_.find(args.subProgramLowerCase()) != cmdToFunc_.end()) {
      const auto &fi = cmdToFunc_.at(args.subProgramLowerCase());
      return fi.func_(args);
    }
    // now try to find program in one of the rings
    for (auto &ring : rings_) {
      if (ring.second->containsProgram(args.subProgramLowerCase())) {
        return ring.second->runProgram(args);
      }
    }
    //if we made it here, the one ring didn't contain the program and none of the sub rings either
    listPrograms(std::cout, args.subProgram_, nameOfProgram_);
    return 1;
  }

  /**@brief A function to check to see if current program contains the
   *sub-program
   *
   * @param program Name of subprogram to search for
   */
  virtual bool containsProgram(const std::string &program) const {
  	for(const auto & ring : rings_){
  		if(ring.second->containsProgram(program)){
  			return true;
  		}
  	}
    return cmdToFunc_.find(program) != cmdToFunc_.end();
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
	virtual void listPrograms(std::ostream &out, const std::string &command,
			const std::string &nameOfProgram) const {
		if (nameOfProgram == nameOfProgram_) {
			out << "Programs" << std::endl;
			out << "Use " << nameOfProgram_
					<< " [PROGRAM] -help to see more details about each program"
					<< std::endl;
			out << "Commands are not case sensitive" << std::endl;
		}
		out << nameOfProgram_ << std::endl;
		listCommands(out);
		for (auto &ring : rings_) {
			ring.second->listPrograms(out, "", nameOfProgram);
		}
		if ("" != command) {
			out << "Unrecognized command " << command << std::endl;
			std::pair<std::string, int> theClosetProgram = closestProgram(command);
			for (auto &ring : rings_) {
				std::pair<std::string, int> currentClosestProgram =
						ring.second->closestProgram(command);
				if (currentClosestProgram.second > theClosetProgram.second) {
					theClosetProgram = currentClosestProgram;
				}
			}
			out << "Input command " << command << std::endl;
			out << "Did you mean  " << theClosetProgram.first << "?" << std::endl;
		}
	}
};

}  // namespace progutils
}  // namespace bib
