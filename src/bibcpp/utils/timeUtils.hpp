#pragma once
/*
 * timeUtils.hpp
 *
 *  Created on: Nov 12, 2014
 *      Author: nickhathaway
 */


#include <time.h> //time_t
#include <chrono>
#include "bibcpp/stdAddition.h" //to_string
#include "bibcpp/utils/utils.hpp" // printTableOrganized(), roundDecPlaces()
#include "bibcpp/utils/stringUtils.hpp"  //leftPadNumStr()

namespace bib{
/**@brief get the current date formated as year.month.day.hours.minutes
 *
 * @return A string of the current date formated
 */
inline std::string getCurrentDate() {
  time_t t = time(0);  // get time now
  struct tm* now = localtime(&t);
  std::stringstream timeStream;
  timeStream << (now->tm_year + 1900) << '-' << leftPadNumStr((now->tm_mon + 1))
             << '-' << leftPadNumStr(now->tm_mday) << '_'
             << leftPadNumStr(now->tm_hour) << '.'
             << leftPadNumStr(now->tm_min);
  return timeStream.str();
}

/**@brief convert string in seconds to a formated time
 *
 * @param timeInSecondsOriginal The time in seconds
 * @param verbose Whether long names for hours,minutes, etc. shoudld be used
 * @param secondsDecimalPlaces the number of decimental places for the seconds output
 * @return
 */
inline std::string getTimeFormat(double timeInSecondsOriginal, bool verbose,
                                 int secondsDecimalPlaces) {
  std::stringstream duration;

  double timeInSeconds = timeInSecondsOriginal;
  if (timeInSeconds > 31536000) {
    int years = (int)timeInSeconds / 31536000;
    if (verbose) {
      duration << "yrs:" << years << ",";
    } else {
      duration << years << ":";
    }
    timeInSeconds = timeInSeconds - years * 31536000.0;
  }
  if (timeInSeconds > 86400) {
    int days = (int)timeInSeconds / 86400;
    if (verbose) {
      duration << "days:" << leftPadNumStr(days, 365) << ",";
    } else {
      duration << days << ":";
    }
    timeInSeconds = timeInSeconds - days * 86400.0;
  } else if (timeInSecondsOriginal > 86400) {
    if (verbose) {
      duration << "days:000,";
    } else {
      duration << "000:";
    }
  }
  if (timeInSeconds > 3600) {
    int hrs = (int)timeInSeconds / 3600;
    if (verbose) {
      duration << "hrs:" << leftPadNumStr(hrs, 24) << ",";
    } else {
      duration << leftPadNumStr(hrs, 24) << ":";
    }
    timeInSeconds = timeInSeconds - hrs * 3600.0;
  } else if (timeInSecondsOriginal > 3600.0) {
    if (verbose) {
      duration << "hrs:00,";
    } else {
      duration << "00:";
    }
  }
  if (timeInSeconds > 60) {
    int minutes = (int)timeInSeconds / 60;
    if (verbose) {
      duration << "mins:" << leftPadNumStr(minutes, 60) << ",";
    } else {
      duration << leftPadNumStr(minutes, 60) << ":";
    }

    timeInSeconds = timeInSeconds - minutes * 60.0;
  } else if (timeInSecondsOriginal > 60) {
    if (verbose) {
      duration << "mins:00,";
    } else {
      duration << "00:";
    }
  }
  duration << std::setprecision(secondsDecimalPlaces + 1);
  if (timeInSeconds > 0) {
    if (timeInSecondsOriginal < 1) {
      if (verbose) {
        duration << "secs:" << roundDecPlaces(timeInSeconds,
                                              secondsDecimalPlaces);
      } else {
        duration << roundDecPlaces(timeInSeconds, secondsDecimalPlaces);
      }
    } else {
      if (verbose) {
        duration << "secs:"
                 << leftPadNumStr(
                        roundDecPlaces(timeInSeconds, secondsDecimalPlaces),
                        60.0);
      } else {
        duration << leftPadNumStr(
                        roundDecPlaces(timeInSeconds, secondsDecimalPlaces),
                        60.0);
      }
    }
  } else {
    if (verbose) {
      duration << "secs:00";
    } else {
      duration << "00";
    }
  }
  return duration.str();
}
namespace sch = std::chrono;

/**@ Get the time in seconds since start
 *
 * @param start The start time to compare against
 * @return Time in seconds
 */
inline double getTimeDiff(sch::time_point<sch::high_resolution_clock> start){
	return sch::duration_cast<sch::nanoseconds>(
			sch::high_resolution_clock::now() - start).count() /
	       static_cast<double>(sch::high_resolution_clock::period::den);
}
/**@brief Get the difference of time in seconds between start and end
 *
 * @param start the start time
 * @param end the end time
 * @return time in seconds
 */
inline double getTimeDiff(sch::time_point<sch::high_resolution_clock> start,
		sch::time_point<sch::high_resolution_clock> end){
	return sch::duration_cast<sch::nanoseconds>(end - start).count() /
	              static_cast<double>(sch::high_resolution_clock::period::den);
}


/**@brief Class for keeping time using the highest resolution clock available from std::chrono
 *
 *\todo Add more raw logging to simply get ticks or nanosecond run information
 *
 */
class stopWatch {
public:

	/**@brief Default constructor to time now
	 *
	 */
	stopWatch():start_(sch::high_resolution_clock::now()),
	currentLap_(sch::high_resolution_clock::now()){
		currentLapName_ = "1";
	}
	/**@brief Construct with the given time point
	 *
	 */
	stopWatch(sch::time_point<sch::high_resolution_clock> startingPoint)
	:start_(startingPoint),
	currentLap_(startingPoint){
		currentLapName_ = "1";
	}
	/**@brief Start time, logging with the highest resolution available
	 *
	 */
	sch::time_point<sch::high_resolution_clock> start_;
protected:
	/**@brief Current time for new lap, logging with the highest resolution available
	 *
	 */
	sch::time_point<sch::high_resolution_clock> currentLap_;
	/**@brief Name of currentLap_
	 *
	 */
	std::string currentLapName_;
	/**@brief A vector of pairs with first a string with lap name and second the lap time, not a map to preserve order
	 *
	 *
	 */
	std::vector<std::pair<std::string, double>> lapTimes_;
public:
	//functions
	/**@brief Get the current lap name
	 *
	 * @return the current lap name
	 */
	std::string getLapName()const{
		return currentLapName_;
	}
	/**@brief Set the current lap name
	 *
	 * @param lapName the new name for the current lap name
	 */
	void setLapName(const std::string & lapName){
		currentLapName_ = lapName;
	}

	/**@brief Get time in seconds since start
	 * @return A double for time in seconds
	 */

	double totalTime(){
		return getTimeDiff(start_);
	}
	/**@brief Get formatted time since start formatted
	 *
	 * @param decPlaces How many decimal places to round to
	 * @return A string with the time formatted for hrs, min, secs etc.
	 */
	std::string totalTimeFormatted(int32_t decPlaces = 6){
		return getTimeFormat(getTimeDiff(start_),true, decPlaces);
	}
	/**@brief Get time in seconds since last point
	 * @return A double for time in seconds
	 */
	double timeLap(){
		return getTimeDiff(currentLap_);
	}
	/**@brief Get formatted time since last point formatted
	 *
	 * @param decPlaces How many decimal places to round to
	 * @return A string with the time formatted for hrs, min, secs etc.
	 */
	std::string timeLapFormatted(int32_t decPlaces = 6){
		return getTimeFormat(getTimeDiff(currentLap_),true, decPlaces);
	}
	/**@brief
	 * @param withName If a name is given, current lap time will be logged with that name
	 */

	/**@brief Start a new lap, log the time since last time point and reset the currentLap_ to now
	 *
	 * @param newLapName A name for the new lap, defaults to current lap number
	 * @param lastLapName A name for the last lap if a change is desired otherwise is logged as currentLapName_;
	 */
	void startNewLap(const std::string & newLapName = "",
			const std::string & lastLapName = ""){
		auto timeNew = sch::high_resolution_clock::now();
		if(lastLapName == ""){
			lapTimes_.emplace_back(currentLapName_, getTimeDiff(currentLap_, timeNew));
		}else{
			lapTimes_.emplace_back(lastLapName, getTimeDiff(currentLap_, timeNew));
		}

		if(newLapName == ""){
			currentLapName_ = estd::to_string(lapTimes_.size() + 1);
		}else{
			currentLapName_ = newLapName;
		}
		currentLap_ = timeNew;
	}
	/**@brief Determine if stopWatch has lap times
	 *
	 * @return return true if lap times were recordered and false if no lap times
	 */
	bool containsLapTimes()const{
		return !lapTimes_.empty();
	}


	/**@brief Log the lap times to out in two columns separated by tab
	 * @param out The std::ostream object to log the times to
	 * @param formatted A bool whether the time should be
	 * @param decPlaces The number of decimal places to print if formatted
	 *
	 */
	void logLapTimes(std::ostream & out,
			bool formatted, int32_t decPlaces,
			bool endLastLap){
		if(endLastLap){
			startNewLap();
		}
		if(formatted){
			std::vector<std::vector<std::string>> content;
			std::vector<std::string> header {"lap", "time"};
			for(const auto & lt : lapTimes_){
				content.emplace_back(std::vector<std::string>{lt.first,
					getTimeFormat(lt.second, true, decPlaces)});
			}
			printTableOrganized(content, header, out);
		}else{
			for(const auto & lt : lapTimes_){
				out << lt.first << "\t" << lt.second << "\n";
			}
		}
	}
};

/**@brief stopWatch class that prints time information on destruction
 *
 */
class scopedStopWatch : public stopWatch {
public:
	/**@brief Default Constructor
	 *
	 * @param message Message to print when the watch dies
	 * @param formated Whether the printed messaged should be formated
	 */
	scopedStopWatch(const std::string message, bool formated = false) :
			stopWatch(), message_(message), formated_(formated), out_(std::cout) {
		setLapName(message);
	}
	/**@brief Construct with a different print location than std::cout
	 *
	 * @param message Message to print when the watch dies
	 * @param out Where to print
	 * @param formated Whether the printed messaged should be formated
	 */
	scopedStopWatch(const std::string message,
			std::ostream & out, bool formated = false) :
			stopWatch(), message_(message), formated_(formated), out_(out) {
		setLapName(message);
	}
	std::string message_;/**> The message to print with the time*/
	bool formated_;/**> Whether to print the time formated */
	/**@brief destrcutor with printing the time
	 *
	 */
	std::ostream & out_;
	~scopedStopWatch(){
		if(lapTimes_.size() == 0){
			startNewLap();
			out_ << message_ << " " << getTimeFormat(lapTimes_.front().second,formated_, 6) << std::endl;;
		}else{
			logLapTimes(out_, formated_, 6, true);
		}
	}
};

} // namespace bib
