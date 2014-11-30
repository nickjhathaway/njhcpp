#pragma once
/*
 * scopedMessage.hpp
 *
 *  Created on: Jul 18, 2014
 *      Author: nickhathaway
 */

#include "bibcpp/bashUtils.h"

namespace bibseq {

/**@b A class that prints a message at construction and a message at destrction for debuging purposes
 *
 */
class scopedMessage {
public:
	/**@b Constructor with start and end message
	 *
	 * @param start The message at the start
	 * @param end The message at the destruction
	 * @param out Where to print messages
	 * @param debug A debug flag to turn off printing to make it easy to turn off messaging
	 */
	scopedMessage(const std::string & start,
			const std::string & end, std::ostream & out,
			bool debug): startMessage_(start),
					endMessage_(end), out_(out), debug_(debug) {
		if(debug_) out_ << bib::bashCT::green << startMessage_ << bib::bashCT::reset << std::endl;
	}
	/**@b Constructor with start message and end message is start_end message
	 *
	 * @param start The start message
	 * @param out Where to print messages
	 * @param debug A debug flag to turn off printing to make it easy to turn off messaging
	 */
	scopedMessage(const std::string & start,
			std::ostream & out, bool debug): startMessage_(start),
					endMessage_(start + "_stop"), out_(out), debug_(debug) {
		if(debug_) out_ << bib::bashCT::green << startMessage_ << bib::bashCT::reset << std::endl;
	}

	//members
	std::string startMessage_; /**Start Message */
	std::string endMessage_; /**End Message */
	std::ostream & out_; /**Where to print messages */
	bool debug_; /**Debug flag to turn off printing  */
	//destructor, print the end message for when the obj goes out of scope
	/**@b destructor with print message
	 *
	 */
	~scopedMessage(){
		if(debug_) out_ << bib::bashCT::red << endMessage_ << bib::bashCT::reset << std::endl;
	}
};

} /* namespace bib */


