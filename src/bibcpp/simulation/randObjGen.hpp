#pragma once
/*
 * randObjectGen.hpp
 *
 *  Created on: Jul 26, 2014
 *      Author: nickhathaway
 */

#include <cppitertools/range.hpp>
#include "bibcpp/simulation/randomGenerator.hpp"
#include "bibcpp/common/misc.hpp"
#include <map>
#include <iostream>
#include <exception>

namespace bib {

/**@brief class for generating a random object from a given vector of objects weighting for their given counts
 *
 */
template<typename T, typename N>
class randObjectGen {
public:
	/**@brief Set up for generating objects evenly
	 *
	 * @param objs The vector of objects to be generating
	 */
	randObjectGen(const std::vector<T> & objs):
	objs_(objs), objCounts_(std::vector<N>(objs.size(),1)),
	likelihoods_(createLikelihood(objs_,objCounts_)){
		std::random_device rd;
		mtGen_.seed(rd());
	}
	/**@brief Set up for generating objects weighting for their counts
	 *
	 * @param objs The vector of objects to be generating
	 * @param counts The counts of the objects given, should be the same length as objects
	 */
	randObjectGen(const std::vector<T> & objs,
			const std::vector<N> & counts):
					objs_(objs), objCounts_(counts),
	likelihoods_(createLikelihood(objs_, objCounts_)){
		std::random_device rd;
		mtGen_.seed(rd());
	}
private:
	/**@brief random generator for numbers
	 *
	 */
	std::mt19937_64 mtGen_;
	/**@brief the objects to be generated
	 *
	 */
	std::vector<T> objs_;
	/**@brief the objects counts
	 *
	 */
	std::vector<N> objCounts_;
	/**@brief the likelihood of the objects given their counts, this will be used to return the randomly generated objects
	 *
	 */
	std::multimap<uint64_t, T, std::less<uint64_t>> likelihoods_;

public:

	/**@brief get const reference to objs
	 *
	 * @return a const reference to objs
	 */
	const std::vector<T> & objs() const{
		return objs_;
	}

	/**@brief get const reference to object counts
	 *
	 * @return a const reference to object counts
	 */
	const std::vector<N> & objCounts() const{
		return objCounts_;
	}

	/**@brief get const reference to likelihoods
	 *
	 * @return a const reference to likelihoods
	 */
	const std::multimap<uint64_t, T, std::less<uint64_t>> & likelihoods() const{
		return likelihoods_;
	}


	/**@brief return a random objects weighting for counts
	 *
	 * @return A random object
	 */
	T genObj(){
	  uint64_t sum = 0;
	  uint64_t rando = mtGen_();
	  for (const auto &likelihood : likelihoods_) {
	    sum += likelihood.first;
	    if (sum > rando) {
	      return likelihood.second;
	    }
	  }
	  return likelihoods_.rbegin()->second;
	}
	/**@brief generated a given number of objects with replacement
	 *
	 * @param num The number of objects to generate
	 * @return The randomly generated objects
	 */
	std::vector<T> genObjs(uint32_t num){
		std::vector<T> ans(num);
		std::generate_n(ans.begin(), num, [&]() { return genObj();});
		return ans;
	}
	/**@brief function for generated the likihood map that is used to generate the random objects
	 *
	 * @param objs The objects to generate
	 * @param counts The counts of the objects to generate, should be same length as objects vector
	 * @return the likelihood map
	 */
	static std::multimap<uint64_t, T, std::less<uint64_t>> createLikelihood(
	    const std::vector<T> &objs, const std::vector<N> &counts){
	  if (counts.size() != objs.size()) {
	  	std::stringstream ss;
	  	ss << __PRETTY_FUNCTION__ << "Error" << std::endl;
	  	ss << "Size of counts differs from size of input objects" << std::endl;
	  	ss << "Size of counts: " << counts.size() << std::endl;
	  	ss << "Size of objs: " << objs.size() << std::endl;
	    throw std::runtime_error{ss.str()};
	  }
	  long double countsSum = std::accumulate<decltype(counts.begin()),N>(counts.begin(), counts.end(), 0);

	  std::multimap<uint64_t, T, std::less<uint64_t>> likelihoods;
	  for (const auto &pos : iter::range(objs.size())) {
	    likelihoods.emplace((std::numeric_limits<uint64_t>::max() / countsSum) * counts[pos], objs[pos]);
	  }
	  return likelihoods;
	}
};



} /* namespace bib */


