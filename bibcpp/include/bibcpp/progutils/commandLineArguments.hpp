#pragma once
//
//  commandLineArguments.hpp
//  sequenceTools
//
//  Created by Nicholas Hathaway on 9/29/13.
//  Copyright (c) 2013 Nicholas Hathaway. All rights reserved.
//

#include "bibcpp/utils.h"
#include "bibcpp/files/fileUtils.hpp"
#include "bibcpp/bashUtils.h"

namespace bib {
namespace progutils {

/**@brief Convert the normal argc and argv arguments into a class enabled for advanced
 *searching
 *
 */
class commandLineArguments {

 public:
  /**@brief Construct from the generic argc and argv c++ arguments
   *
   */
  commandLineArguments(int argc, char* argv[])
      : arguments_(convertCommandLineArguments(argc, argv)) {
    // log the arguments from the commandline as
    std::stringstream hold;
    logRunArgumnets(hold, argc, argv);
    commandLine_ = hold.str();
    arguments_["-commandline"] = commandLine_;
  }
  /**@brief Construct from a map of commands, key is flag and value is flag associated
   *value
   *
   */
  commandLineArguments(const std::map<std::string, std::string>& inputCommands)
      : arguments_(inputCommands) {
    lookForOption(commandLine_, "-commandline");
  }

  // members
  /**@brief A map of the commandline arguments, key is flag and value is flag
   *associated value
   *
   */
  std::map<std::string, std::string> arguments_;
  /**@brief A string containing what the command was from the commandline and where it
   *was executed
   *
   */
  std::string commandLine_;

 private:
  std::string& lowerCaseGetArguments(const std::string& str) {
    if (arguments_[strToLowerRet(str)] == "") {
      std::cout << "Argument " << str
                << " requires an option but none was given" << std::endl;
      exit(1);
    }
    return arguments_[strToLowerRet(str)];
  }

 public:
  /**@brief Accessor to get the vaue associated with flag by case insensitive look up
   *@param str The flag being looked up, should be preceed with -, eg.
   *-exampleFlag
   *@return A string reference to the value associated with the flag
   */
  std::string& operator[](const std::string& str) {
    return lowerCaseGetArguments(str);
  }

  // string
  /** @brief Set option for string option
   * Set the string to exactly what the following option the flag was
   * @param option Reference to string option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   * case insensitive
   */
  bool lookForOption(std::string& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = lowerCaseGetArguments(flag);
      return true;
    } else {
      return false;
    }
  }
  // bool
  /** @brief Set option for bool option
   * If flag is present the given bool option is set to true, otherwise left on
   * touched
   * @param option Reference to bool option to be set to true
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   * case insensitive
   */
  bool lookForOption(bool& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = true;
      return true;
    } else {
      return false;
    }
  }
  // bool false
  /** @brief Set option for setting bool option to false
   * If flag is present the given bool option is set to false, otherwise left on
   * touched
   * @param option Reference to bool option to be set to false
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   * case insensitive
   */
  bool lookForOptionFalse(bool& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = false;
      return true;
    } else {
      return false;
    }
  }
  // int16_t
  /** @brief Set option for setting int16_t options
   * Set given option to the output of std::stoi of value associated with the
   *given flag
   *
   * @param option Reference to int16_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(int16_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = estd::stos(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // int32_t
  /** @brief Set option for setting int32_t options
   * Set given option to the output of std::stoi of value associated with the
   *given flag
   *
   * @param option Reference to int32_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(int32_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stoi(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // int64_t
  /** @brief Set option for setting int64_t options
   * Set given option to the output of std::stol of value associated with the
   *given flag
   *
   * @param option Reference to int64_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(int64_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stoll(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // uint16_t
  /** @brief Set option for setting uint16_t options
   * Set given option to the output of std::stoull of value associated with the
   *given flag
   *
   * @param option Reference to uint16_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(uint16_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = estd::stous(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // uint32_t
  /** @brief Set option for setting uint32_t options
   * Set given option to the output of own version of std::stou of value
   *associated with the given flag
   *
   * @param option Reference to uint32_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(uint32_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = estd::stou(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // uint64_t
  /** @brief Set option for setting uint64_t options
   * Set given option to the output of own version of std::stoull of value
   *associated with the given flag
   *
   * @param option Reference to uint64_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(uint64_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stoull(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
#ifndef __linux__
  // size_t
  /** @brief Set option for setting size_t options
   * Set given option to the output of std::stoul of value associated with the
   *given flag
   *
   * @param option Reference to size_t option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(size_t& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stoull(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
#endif
  // double
  /** @brief Set option for setting double options
   * Set given option to the output of std::stod of value associated with the
   *given flag
   *
   * @param option Reference to double option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(double& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stod(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // long double
  /** @brief Set option for setting long double options
   * Set given option to the output of std::stold of value associated with the
   *given flag
   *
   * @param option Reference to long double option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(long double& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stold(lowerCaseGetArguments(flag));
      return true;
    } else {
      return false;
    }
  }
  // float
  /** @brief Set option for setting float options
   * Set given option to the output of std::stof of value associated with the
   *given flag
   *
   * @param option Reference to float option to be set
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent, search is
   *case insensitive
   */
  bool lookForOption(float& option, const std::string& flag) {
    if (containsFlagCaseInsensitive(flag)) {
      option = std::stof(lowerCaseGetArguments(flag));
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
  bool containsFlagCaseSensitive(const std::string& flag) {
    if (arguments_.find(flag) != arguments_.end()) {
      return true;
    } else {
      return false;
    }
  }
  /** @brief Check to see if flag, case insensitive
   * Checks for the present of a flag ignoring case of letters
   *
   * @param flag The flag to be searched for for current option
   * @return Bool, true if flag is present or false if flag is absent
   */
  bool containsFlagCaseInsensitive(const std::string& flag) {
    if (arguments_.find(strToLowerRet(flag)) != arguments_.end()) {
      return true;
    } else {
      return false;
    }
  }
  /** @brief Static function for logging commandline arguments in one string
   * Takes the generic argc and argv and logs them as is and prints the
   *directory from where they were executed
   *
   * @param out The std::ostream object to log the info to
   * @param argc The number of arguments
   * @param argv The array of char pointers of the arguments
   */
  static void logRunArgumnets(std::ostream& out, int argc, char* argv[]) {
    std::string directory = files::get_cwd();
    out << "Command ran from " << directory << std::endl;
    for (int args = 0; args < argc; ++args) {
      out << argv[args] << " ";
    }
    out << "\n";
  }

  /** @brief Static function for logging commandline arguments in one string
   * Takes the generic argc and argv and logs them as is and prints the
   *directory from where they were executed
   *
   * @param out The std::ostream object to log the info to
   * @param argc The number of arguments
   * @param argv The array of char pointers of the arguments
   */
  static void logRunArgumnets(std::ostream& out, std::map<std::string, std::string> commands) {
    std::string directory = files::get_cwd();
    out << "Command ran from " << directory << std::endl;
    out << commands["-program"];

    for (auto com : commands) {
    	if(com.first != "-program" && com.first != "" && com.first != "-commandline"){
    		 out << com.first << " ";
    		 if(com.second != ""){
    			 out << com.first << " ";
    		 }
    	}
    }
    out << "\n";
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
  static std::map<std::string, std::string> convertCommandLineArguments(int argc, char* argv[]) {
    std::map<std::string, std::string> storage;
    for (int i = 1; i < argc; i++) {
      std::string nextParamRaw = argv[i];
      uint32_t equCount =
          std::count(nextParamRaw.begin(), nextParamRaw.end(), '=');
      if (equCount > 0) {
        if (equCount > 1) {
          std::cout << bashCT::addColor(196) << std::endl;
          std::cout << "Error can only have one equal sign when setting flag"
                    << std::endl;
          std::cout << "eg. -flag=this , can't have -flag=this=that"
                    << std::endl;
          std::cout << "not " << nextParamRaw << bashCT::reset
                    << std::endl;
          exit(1);
        } else {
          std::vector<std::string> toks = tokenizeString(nextParamRaw, "=");
          storage.insert(std::make_pair(strToLowerRet(toks[0]), toks[1]));
        }
      } else {
        std::string nextParam = strToLowerRet(argv[i]);
        if (storage.find(nextParam) != storage.end()) {
          std::cout << "Error, already have " << nextParam << std::endl;
          std::cout << "Check if you entered it in twice" << std::endl;
          exit(1);
        }

        if (argv[i][0] == '-') {
          if (i + 1 >= argc) {
            storage.insert(std::make_pair(strToLowerRet(argv[i]), ""));
          } else {
            if (argv[i + 1][0] == '-') {
              storage.insert(std::make_pair(strToLowerRet(argv[i]), ""));
            } else {
              storage.insert(
                  std::make_pair(strToLowerRet(argv[i]), argv[i + 1]));
              ++i;
            }
          }
        } else {
          storage.insert(std::make_pair(strToLowerRet(argv[i]), ""));
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
    int offSet = 0;
    if (arguments_.find("-program") != arguments_.end()) {
      ++offSet;
    }
    if (arguments_.find("-commandline") != arguments_.end()) {
      ++offSet;
    }
    return arguments_.size() - offSet;
  }
};
}  // namespace progutils

} // namespace bib
