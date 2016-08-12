#pragma once
/*
 * LockableQueue.hpp
 *
 *  Created on: Jan 17, 2016
 *      Author: nick
 */


#include <queue>
#include <mutex>


namespace bib {
namespace concurrent {

/**@brief A class to wrap queue and mutex to give the ability to lock the queue
 *
 */
template<typename T>
class LockableQueue {
	std::mutex mut_; /**< a mutex to enable locking */
	std::queue<T> vals_; /**< a queue to hold values */
public:
	/**@brief construct with a deque
	 *
	 * @param vals a deque with values to be able to pop while locking
	 */
	LockableQueue(const std::deque<T> & vals) :
			vals_ { vals } {
	}
	/**@brief construct with a vector
	 *
	 * @param vals a vector with values to be able to pop while locking
	 */
	LockableQueue(const std::vector<T> & vals) :
			LockableQueue(std::deque<T> { vals.begin(), vals.end() }) {
	}

	/**@brief set the value for val object, lock the class to prevent simultaneous access
	 *
	 * @param val a value to set with the next object
	 * @return whether or not there was another object to pop
	 */
	bool getVal(T & val) {
		std::lock_guard<std::mutex> lock(mut_);
		if (!vals_.empty()) {
			val = vals_.front();
			vals_.pop();
			return true;
		}
		return false;
	}
};

}  // namespace concurrent
}  // namespace bib
