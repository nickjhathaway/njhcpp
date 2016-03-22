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

namespace bib {
namespace concurrent {
template<typename T>
class LockableVec{
	std::mutex mut_;
	std::vector<T> vals_;
	std::atomic<size_t> indx_{0};
public:
	template<typename CON>
	LockableVec(const CON & vals):vals_{vals.begin(), vals.end()}{}

	bool getVal(T & val){
		auto pos = indx_.fetch_add(1, std::memory_order_relaxed);
		if(pos < vals_.size()){
			val = vals_[pos];
			return true;
		}
		return false;
	}
};

}  // namespace concurrent
}  // namespace bib


