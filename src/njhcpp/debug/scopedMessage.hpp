#pragma once
/*
 * scopedMessage.hpp
 *
 *  Created on: Jul 18, 2014
 *      Author: nickhathaway
 */

#include "njhcpp/bashUtils.h"

namespace njh {

/**@brief A class that prints a message at construction and a message at deconstruction for debuging purposes
 *
 */
class scopedMessage {
public:
	/**@brief Constructor with start and end message
	 *
	 * @param start The message at the start
	 * @param end The message at the destruction
	 * @param out Where to print messages
	 * @param debug A debug flag to turn off printing to make it easy to turn off messaging
	 */
	scopedMessage(const std::string & start, const std::string & end,
			std::ostream & out, bool debug, uint32_t indentLvl = 0) :
			startMessage_(start), endMessage_(end), out_(out), debug_(debug), indentLvl_(
					indentLvl) {
		if (debug_) {
			if (indentLvl_ > 0) {
				out_ << std::string(indentLvl_, '\t');
			}
			out_ << njh::bashCT::green << startMessage_ << njh::bashCT::reset
					<< std::endl;
		}
	}
	/**@brief Constructor with start message and end message is start_end message
	 *
	 * @param start The start message
	 * @param out Where to print messages
	 * @param debug A debug flag to turn off printing to make it easy to turn off messaging
	 */
	scopedMessage(const std::string & start, std::ostream & out, bool debug,
			uint32_t indentLvl = 0) :
			startMessage_(start), endMessage_(start + "_stop"), out_(out), debug_(
					debug), indentLvl_(indentLvl) {
		if (debug_) {
			if (indentLvl_ > 0) {
				out_ << std::string(indentLvl_, '\t');
			}
			out_ << njh::bashCT::green << startMessage_ << njh::bashCT::reset
					<< std::endl;
		}
	}

	//members
	std::string startMessage_; /**< Start Message */
	std::string endMessage_; /**< End Message */
	std::ostream & out_; /**< Where to print messages */
	bool debug_; /**< Debug flag to turn off printing  */

	uint32_t indentLvl_ = 0 /**< How much to indent for the message */;

	/**@brief destructor with print message
	 *
	 * print the end message for when the obj goes out of scope
	 */
	~scopedMessage(){
		if(debug_){
			if(indentLvl_ > 0){
				out_ << std::string(indentLvl_, '\t');
			}
			out_ << njh::bashCT::blue << endMessage_ << njh::bashCT::reset << std::endl;
		}
	}
};

} /* namespace njh */


