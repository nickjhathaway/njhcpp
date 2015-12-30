#pragma once
/*
 * timeUtils.hpp
 *
 *  Created on: Nov 12, 2014
 *      Author: nickhathaway
 */


#include <time.h> //time_t
#include <chrono>

#include "bibcpp/common.h" //to_string
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



} // namespace bib
