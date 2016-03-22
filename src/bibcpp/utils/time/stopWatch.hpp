#pragma once
/*
 * stopWatch.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */


#include <chrono>
#include "bibcpp/common.h" //to_string
#include "bibcpp/utils/time/timeUtils.hpp"

namespace bib {
namespace sch = std::chrono;

/**@brief Class for keeping time using the highest resolution clock available from std::chrono
 *
 *@todo Add more raw logging to simply get ticks or nanosecond run information
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

}  // namespace bib


