#pragma once
/*
 * ProgressBar.hpp
 *
 *  Created on: Jun 9, 2015
 *      Author: nick
 */

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip> //set::setw
#include <algorithm>
#include <cppitertools/range.hpp> //range
#include <cmath>
#include <atomic>

#include "njhcpp/bashUtils/textFormatter.hpp"
#include "njhcpp/utils/time/stopWatch.hpp"

namespace njh {

/**@brief A little play class for printing a progress bar
 *
 */
class ProgressBar {
public:
	/**@brief construct with total number to count to
	 *
	 * @param total The total that the progress should go to
	 */
	ProgressBar(uint32_t total) :
			total_(total) {
		step_ = width_ / 10;
		progColors_ = RdYlGn_;
	}
	/**@brief output current progress by adding the current value stored
	 *
	 * @param out the stream to output a progress string to
	 * @param add the amount to add to the current value
	 * @param showTime whether to print the time with an eta as well
	 */
	void outputProgAdd(std::ostream & out, uint32_t add, bool showTime = false) {
		current_ = atCur_.fetch_add(add) + add;
		if ( std::round((current_ / static_cast<double>(total_)) * width_) > oldtProg_ || current_ == total_) {
			std::lock_guard<std::mutex> lock(mut_);
			if (progressNoLock()) {
				out << "\r" << getOutStr(showTime);
				out.flush();
				if (current_ == total_) {
					std::cout << std::endl;
				}
			}
		}
	}

	std::map<uint32_t, std::string> progColors_;
	std::map<uint32_t, std::string> readToGreen_ = { { 0, njh::bashCT::addBGColor(
			196) }, { 1, njh::bashCT::addBGColor(199) }, { 2, njh::bashCT::addBGColor(
			201) }, { 3, njh::bashCT::addBGColor(129) }, { 4, njh::bashCT::addBGColor(
			57) }, { 5, njh::bashCT::addBGColor(27) }, { 6, njh::bashCT::addBGColor(
			39) }, { 7, njh::bashCT::addBGColor(45) }, { 8, njh::bashCT::addBGColor(
			50) }, { 9, njh::bashCT::addBGColor(46) } };
	std::map<uint32_t, std::string> RdYlGn_ = {
			{ 0, njh::bashCT::addBGColor(124) }, { 1, njh::bashCT::addBGColor(167) },
			{ 2, njh::bashCT::addBGColor(209) }, { 3, njh::bashCT::addBGColor(215) },
			{ 4, njh::bashCT::addBGColor(222) }, { 5, njh::bashCT::addBGColor(229) },
			{ 6, njh::bashCT::addBGColor(149) }, { 7, njh::bashCT::addBGColor(71) }, {
					8, njh::bashCT::addBGColor(78) }, { 9, njh::bashCT::addBGColor(29) } };
private:

	/**@brief to be called after updated current_
	 *
	 * @return whether the progress string was updated and needs to be written
	 */
	bool progressNoLock() {
		if (current_ > total_) {
			std::cerr << std::endl << __PRETTY_FUNCTION__ << ": Error, currently held value: " << current_
					<< ", has progressed further than total: " << total_ << std::endl;
			return false;
		}
		if (1 == current_) {
			watch_.reset();
		}
		//set current progress
		currentProgress_ = current_ / static_cast<double>(total_);
		tprog_ = std::round(currentProgress_ * width_);
		if (tprog_ == oldtProg_ && current_ < total_) {
			return false;
		}
		//set new time
		watch_.startNewLap();
		oldtProg_ = tprog_;
		progStr_ = "";
		for (auto i : iter::range(tprog_ / step_ + 1)) {
			progStr_.append(progColors_[i]);
			progStr_.append(std::string(std::min(step_, tprog_ - i * step_), ' '));
		}
		progStr_.append(njh::bashCT::reset);
		padStr_ = njh::bashCT::addBGColor(188) + std::string(width_ - tprog_, ' ')
				+ njh::bashCT::reset + " ";
		return true;
	}
	/**@brief get the current progress line
	 *
	 * @param showTime Whether to show duration and ETA
	 *
	 * @return the progress line
	 */
	std::string getOutStr(bool showTime = false) const {
		std::string add = "";
		if (showTime) {
			add = ":dur: " + getTimeFormat(watch_.totalTime(), true, 0);
			if (tprog_ > 1) {
				add += "; ETA: "
						+ getTimeFormat(watch_.getAverageLapTime() * (width_ - tprog_),
								true, 0) + std::string(10, ' ');
			}
		}
		return progStr_ + padStr_ + getPerStr() + add;
	}
	/**@brief get the percentage string of the current progress
	 *
	 * @return
	 */
	std::string getPerStr() const {
		std::stringstream progSs;
		progSs << njh::bashCT::bold << static_cast<uint32_t>(currentProgress_ * 100)
				<< "% " << njh::bashCT::reset << current_;
		return progSs.str();
	}

	stopWatch watch_;

	std::atomic<uint32_t> atCur_{0};
	uint32_t current_ = 0;
	uint32_t total_;
	uint32_t tprog_ = 0;
	uint32_t width_ = 50;
	uint32_t step_;
	uint32_t oldtProg_ = 0;
	double currentProgress_ = 0;

	std::mutex mut_;

	std::string progStr_;
	std::string padStr_;
};

} //namespace njh

