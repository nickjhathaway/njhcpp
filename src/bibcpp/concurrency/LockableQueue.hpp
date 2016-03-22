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


template<typename T>
class LockableQueue{
	std::mutex mut_;
	std::queue<T> vals_;
public:
	LockableQueue(const std::deque<T> & vals): vals_{vals}{}
	LockableQueue(const std::vector<T> & vals): LockableQueue(std::deque<T> {vals.begin(), vals.end()}){}

	bool getVal(T & val){
		std::lock_guard<std::mutex> lock(mut_);
		if(!vals_.empty()){
			val = vals_.front();
			vals_.pop();
			return true;
		}
		return false;
	}
};

}  // namespace concurrent
}  // namespace bib
