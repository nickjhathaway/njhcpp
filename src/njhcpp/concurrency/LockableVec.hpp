#pragma once
/*
 * LockableVec.hpp
 *
 *  Created on: Feb 12, 2016
 *      Author: nick
 */

#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>

namespace njh {
namespace concurrent {

/**@brief a vector with concurrent access done by an atomic indexer
 *
 * objects will not be removed from vector, just the current index changes
 *
 */
template<typename T>
class LockableVec {
	std::vector<T> vals_; /**< values to hold and enable access while locking*/
	std::atomic<size_t> indx_ { 0 }; /**< the current index in vals*/
public:

	/**@brief construct with another container
	 *
	 * @param vals a conainer with values to hold
	 */
	template<typename CON>
	LockableVec(const CON & vals) :
			vals_ { vals.begin(), vals.end() } {
	}

	/**@brief get a value and changing indx_
	 *
	 * @param val a value to set
	 * @return whether there was another value to get
	 */
	bool getVal(T & val) {
		auto pos = indx_.fetch_add(1, std::memory_order_relaxed);
		if (pos < vals_.size()) {
			val = vals_[pos];
			return true;
		}
		return false;
	}

	/**@brief Fill a vector with a set number of values
 *
 * @param vals the vector to fill with number of values
 * @param number the number to attemtp to extract, will less than this is left in the vector, will only give what's left
 * @return if any vals were extracted
 */
	bool getVals(std::vector<T> & vals, const uint32_t number) {
		vals.clear();
		vals.reserve(number);
		auto pos = indx_.fetch_add(1, std::memory_order_relaxed);
		if (pos < vals_.size()) {
			vals.emplace_back(vals_[pos]);
			while (pos < vals_.size() && vals.size() < number) {
				pos = indx_.fetch_add(1, std::memory_order_relaxed);
				vals.emplace_back(vals_[pos]);
			}
			return true;
		}
		return false;
	}

	/**@brief reset the index to zero
	 *
	 */
	void reset() {
		indx_.store(0, std::memory_order_relaxed);
	}
};

}  // namespace concurrent
}  // namespace njh


