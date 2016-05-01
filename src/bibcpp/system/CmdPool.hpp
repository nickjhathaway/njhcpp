#pragma once
/*
 * CmdPool.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: nick
 */

#include "bibcpp/common.h"
#include <mutex>
#include <atomic>
#include "bibcpp/jsonUtils/jsonUtils.hpp"

namespace bib {
namespace sys {



/**@brief Class to hold a pool of system commands with a lock so threads can just pull from this
 *
 */
template<typename T>
class CmdPool {
public:
	/**@brief Construct with a vector of commands to run
	 *
	 * @param cmds The vector of commands
	 */
	CmdPool(const std::vector<T> & cmds): cmds_(cmds){

	}
private:
	std::vector<T> cmds_; /**< The commands to run */
	uint32_t currentCmd_ = 0; /**< Index of what command to run next */

	std::mutex mut_; /**< Mutex for locking pool while thread is grabbing command */
public:
	/**@brief small class to hold whether there is a next class and what it is
	 *
	 */
	struct cmdStatus{
		T cmd_;
		bool valid_ ;
	};
	/**@brief grab next command to run
	 *
	 * @return A struct with whether there is a command to run and a string of what that command is, if invalid string will be blank
	 */
	cmdStatus getCmd(){
		std::lock_guard<std::mutex> lock(mut_);
		if(currentCmd_ < cmds_.size()){
			++currentCmd_;
			return {cmds_ [currentCmd_ - 1],true};
		}else{
			return {T{},false};
		}
	}
};

}  // namespace sys
}  // namespace bib
