#pragma once
/*
 * LockableQueue.hpp
 *
 *  Created on: Jan 17, 2016
 *      Author: nick
 */


#include <queue>
#include <mutex>


namespace njh {
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

	/**@brief construct any container with a begin and end
	 *
	 * @param vals a container with values to be able to pop while locking
	 */
	template<typename C>
	LockableQueue(const C & vals) :
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

	/**@brief Get a chunk of values, less locking
	 *
	 * @param vals the vector to fill with several values from the queue
	 * @param num the number of vals to get
	 * @return whether any values were gotten
	 */
	bool getVals(std::vector<T> & vals, uint32_t num) {
		vals.clear();
		std::lock_guard<std::mutex> lock(mut_);
		if (!vals_.empty()) {
			uint32_t count = 0;
			while(!vals_.empty() && count < num){
				vals.emplace_back(vals_.front());
				vals_.pop();
				++count;
			}
			return true;
		}
		return false;
	}
};

}  // namespace concurrent
}  // namespace njh
