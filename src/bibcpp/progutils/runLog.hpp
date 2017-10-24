#pragma once
/*
 * runLog.hpp
 *
 *  Created on: Aug 2, 2014
 *      Author: nickhathaway
 */

#include "bibcpp/common.h"
#include "bibcpp/utils.h"
#include "bibcpp/files/fileSystemUtils.hpp"
#include "bibcpp/progutils/CmdArgs.hpp"

namespace bib {
namespace progutils {
/**@brief runLog class to help log run info
 *
 */
class RunLog {
 public:
  /**@brief Constructor with the runLog filename and start time as now
   *
   * @param fileName The name of the runLog file to log the information to
   *
   */
  RunLog(const std::string& fileName)
      : fileName_(fileName) {
    files::openTextFile(runLogFile_, fileName_, ".txt", true, false);
  }

  /**@brief Constructor with the runLog filename and the given start
   *
   * @param fileName The name of the runLog file to log the information to
   * @param start Use the given start time for time start
   *
   */
  RunLog(const std::string& fileName,
         std::chrono::time_point<std::chrono::high_resolution_clock> start)
      : fileName_(fileName), timer_(start) {
    files::openTextFile(runLogFile_, fileName_, ".txt", true, false);
  }

  /**@brief Empty constructor, file won't be opened until setFilenameAndOpen() is
   *called
   *
   */
  RunLog() {}

 private:
  // Members
  /**@brief Name of the runLog file
   *
   */
  std::string fileName_;

  /**@brief The start time stored as a stopWatch object
   *
   */
  stopWatch timer_;

 public:
  /**@brief set the current lap name
   *
   * @param str The name for the current lap
   */
  void setCurrentLapName(const std::string & str){
  	timer_.setLapName(str);
  }
  /**@brief The std::ofstream object to log the info to
   *
   */
  std::ofstream runLogFile_;

  /**@brief Open the runLog file with this name, and the start time will be now, will
   *fail if runLog already opened
   *@param fileName The filename for the runLog
   */
  void setFilenameAndOpen(const std::string& fileName) {
    if (runLogFile_.is_open()) {
    	std::stringstream ss;
      ss << "\033[1;31m";
      ss << "in runLog.setFilenameAndOpen()\n";
      ss << "Run log already started\n";
      ss << "\033[0m";
      throw std::runtime_error{ss.str()};
    } else {
      fileName_ = fileName;
      files::openTextFile(runLogFile_, fileName_, ".txt", true, false);
      timer_ = stopWatch();
    }
  }

  /**@brief Open the runLog file with this name and given start time, will fail if
   * runLog already opened
   * @param fileName The filename for the runLog
   * @param start Use given start time for logging time
   */
  void setFilenameAndOpen(
      const std::string& fileName,
      std::chrono::time_point<std::chrono::high_resolution_clock> start) {
    if (runLogFile_.is_open()) {
    	std::stringstream ss;
      ss << "\033[1;31m";
      ss << "in runLog.setFilenameAndOpen()\n";
      ss << "Run log already started\n";
      ss << "\033[0m";
      throw std::runtime_error{ss.str()};
    } else {
      fileName_ = fileName;
      files::openTextFile(runLogFile_, fileName_, ".txt", true, false);
      timer_ = stopWatch(start);
    }
  }

  /**@brief Put starting stamp on runLog by putting date and the command given for the
   *program, will fail if runLog not started
   *
   * @param inputCommands The commands for the current program that contains the
   *commandline for logging
   *
   */
  void startRunLog(const CmdArgs & inputCommands) {
    if (runLogFile_.is_open()) {
      runLogFile_ << "Ran on: " << getCurrentDate() << std::endl;
      runLogFile_ << "Ran from: " << inputCommands.workingDir_ << std::endl;
      runLogFile_ << inputCommands.commandLine_ << std::endl;
    } else {
      std::cerr << "\033[1;31m";
      std::cerr << "in runLog.startRunLog()\n";
      std::cerr << "RunLog object's runLogfile not open yet\n";
      std::cerr << "while attempting start runLog\033[0m\n";
      std::cerr << "\033[0m";
    }
  }

  /**@brief Log the current time difference from when the time point start_, will fail
   *if runLog not started
   *
   */
  void logTotalTime(uint32_t decPlaces = 6) {
    if (runLogFile_.is_open()) {
      runLogFile_ << timer_.totalTimeFormatted(decPlaces) << "\n";
    } else {
      std::cerr << "\033[1;31m";
      std::cerr << "in runLog.logTime()\n";
      std::cerr << "RunLog object's runLogfile not open yet\n";
      std::cerr << "while attempting to log time\n";
      std::cerr << "\033[0m";
    }
  }
  /**@brief Log the current time for the current lap information and start new lap
   *
   * @param timeNewName A new name for the next lap
   * @param decPlaces The number of decimal places to have
   */
  void logCurrentTime(const std::string & timeNewName = "",
  		uint32_t decPlaces = 6) {
    if (runLogFile_.is_open()) {
    	runLogFile_ << timer_.getLapName() << " time: "
    			<< timer_.timeLapFormatted(decPlaces) << "\n";
      runLogFile_ << "total time: " << timer_.totalTimeFormatted(decPlaces)
      		<< "\n";
    	timer_.startNewLap(timeNewName);
    } else {
      std::cerr << "\033[1;31m";
      std::cerr << "in runLog.logTime()\n";
      std::cerr << "RunLog object's runLogfile not open yet\n";
      std::cerr << "while attempting to log time\n";
      std::cerr << "\033[0m";
    }
  }

  /**@brief Destructor, log the the total time if a runLog is open
   *
   */
  ~RunLog() {
  	if(runLogFile_.is_open()){
  		if(!timer_.containsLapTimes()){
  			logTotalTime(6);
  		}else{
  			timer_.logLapTimes(runLogFile_, true, 6, true);
  			runLogFile_ << "total time: "; logTotalTime(6);
  		}
  	}
  }
};

/**@brief Input operator to log information to the runLogFile_ for the given runLog,
 *will fail if runLog not started
 *
 */
template <typename T>
RunLog& operator<<(RunLog& rLog, const T& obj) {
  if (rLog.runLogFile_.is_open()) {
    rLog.runLogFile_ << obj;
  } else {
    std::cerr << "\033[1;31m";
    std::cerr << "RunLog object's runLogfile not open yet\n";
    std::cerr << "while attempting to use << operator on runLog\n";
    std::cerr << "\033[0m";
  }
  return rLog;
}

} // namespace progutils

} // namespace bib
