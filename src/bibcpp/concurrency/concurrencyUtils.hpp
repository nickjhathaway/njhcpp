#pragma once

/*
 * concurrencyUtils.hpp
 *
 *  Created on: Aug 6, 2017
 *      Author: nick
 */


#include "bibcpp/common.h"
#include <thread>

namespace bib {
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


} // namespace concurrent
} // namespace bib


