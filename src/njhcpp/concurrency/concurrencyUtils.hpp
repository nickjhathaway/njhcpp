#pragma once

/*
 * concurrencyUtils.hpp
 *
 *  Created on: Aug 6, 2017
 *      Author: nick
 */


#include "njhcpp/common.h"

#include <thread>
#include <functional>

namespace njh {
namespace concurrent {

/**@brief Join all threads in the threads vectors, could throw if a thread isn't joinable
 *
 * @param threads a vector of threads objects, all should be joinable
 */
inline void joinAllThreads(std::vector<std::thread> & threads){
	for_each(threads, std::mem_fn(&std::thread::join));
}

/**@brief Join all joinable threads
 *
 * @param threads a vector of thread objects to join, will be checked for joinable first
 */
inline void joinAllJoinableThreads(std::vector<std::thread> & threads){
	for_each(threads, [](std::thread & t ){
		if(t.joinable()){
			t.join();
		}
	});
}

/**@brief Detach all threads in the threads vector, could throw if a thread isn't joinable
 *
 * @param threads a vector of threads objects, all should be joinable
 */
inline void detachAllThreads(std::vector<std::thread> & threads){
	for_each(threads, std::mem_fn(&std::thread::detach));
}

/**@brief Detach all joinable threads in the threads vector
 *
 * @param threads a vector of thread objects to join, will be checked for joinable first
 */
inline void detachAllJoinableThreads(std::vector<std::thread> & threads){
	for_each(threads, [](std::thread & t ){
		if(t.joinable()){
			t.detach();
		}
	});
}


/**@brief Run a function that takes no arguments and returns nothing over a number of threads
 *
 * @param func the fuction object to run, pass by reference in case it has object references that need to be updated
 * @param numThreads the number of threads to use
 */
inline void runVoidFunctionThreaded(std::function<void()> & func, uint32_t numThreads){
	std::vector<std::thread> threads;
	for(uint32_t t = 0; t < numThreads; ++t){
		threads.emplace_back(std::thread(func));
	}
	joinAllThreads(threads);
}




} // namespace concurrent
} // namespace njh


