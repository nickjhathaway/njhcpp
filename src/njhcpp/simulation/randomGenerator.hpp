#pragma once
/*
 * randomGenerator.hpp
 *
 *  Created on: Nov 13, 2014
 *      Author: nickhathaway
 */

#include <random>
#include <sstream>
#include "njhcpp/common.h"
#include "njhcpp/utils/vecUtils.hpp"

namespace njh {
/**@brief A random generator class that basically just wraps std::mt19937_64 to make generation or random selections a bit easier
 *
 */
class randomGenerator {

public:
	/**@brief default constructor seeds the std::mt19937_64 with a seed from std::random_device
	 *
	 */
	randomGenerator() {
		seed();
	}
	/**@brief constructor with a specific seed so you can repeat random generations
	 *
	 * @param givenSeed
	 */
	randomGenerator(uint64_t givenSeed) {
		seedNum(givenSeed);
	}
	/**@brief the main work horse of the class that generators the base random numbers needed
	 *
	 */
	std::mt19937_64 mtGen_;
	/**@brief the current seed being used for the state of mtGen_
	 *
	 */
	uint64_t currentSeed_;
	/**@brief Generator a random nuber between 0 and 1 in the range [0,1)
	 *
	 * @return a double in [0,1)
	 */
	double unifRand() {
		return mtGen_() / (static_cast<double>(mtGen_.max()) + 1);
	}
	/**@brief Calls unifRand() so you don't have to type the whole thing
	 *
	 * @return a double in [0,1)
	 */
	double operator()() {
		return unifRand();
	}

	/**@brief Get a vector of doubles in [0,1)
	 *
	 * @param num The number of random numbers to generate
	 * @return A vector of double in [0,1)
	 */
	std::vector<double> unifRandVector(uint32_t num) {
		std::vector<double> ret(num);
		std::generate(ret.begin(), ret.end(), [&]() {
			return unifRand();});
		return ret;
	}
	/**@brief Generator numbers in [start,stop), templated so integers or double can be used
	 *
	 * @param start The start of the range inclusive
	 * @param stop The end of the range, exclusive
	 * @return A random number in [start,stop)
	 */
	template<typename T>
	T unifRand(T start, T stop) {
		return static_cast<T>((stop - start) * unifRand()) + start;
	}
	/**@brief Get a vector of random numbers in [start,stop)
	 *
	 * @param start The start of the range inclusive
	 * @param stop The end of the range exclusive
	 * @param num the amount of numbers to generate
	 * @return A vector of numbers in [start,stop)
	 */
	template<typename T>
	std::vector<T> unifRandVector(T start, T stop, int num) {
		std::vector<T> ret(num);
		std::generate_n(ret.begin(), num, [&]() {return unifRand(start, stop);});
		return ret;
	}
	/**@brief A vector of vectors of random numbers in [start,stop)
	 *
	 * @param start The start of the range inclusive
	 * @param stop The end of the range exclusive
	 * @param totalNum the total number of vectors to have
	 * @param subNum the number of random numbers in each vector
	 * @return A vector of vectors of numbers in [start,stop)
	 */
	template<typename T>
	std::vector<std::vector<T>> unifRandVecVec(T start, T stop, uint32_t totalNum,
			uint32_t subNum) {
		std::vector<std::vector<T>> ret(totalNum);
		std::generate(ret.begin(), ret.end(),
				[&]() {return unifRandVector<T>(start,stop,subNum);});
		return ret;
	}
	/**@brief Get a random selection from a vector of objects
	 *
	 * @param vec The vector to sample from
	 * @return A random selection from vec
	 */
	template<typename T>
	T unifRandSelection(const std::vector<T> & vec) {
		return vec[unifRand<uint64_t>(0, vec.size())];
	}
	/**@brief Randomly sample form from a vector
	 *
	 * @param vec the vector from which to sample
	 * @param amt The number of samples to take
	 * @param withReplacement Whether the sample should be done with replacement
	 * @return A random sample from vec
	 */
	template<typename T>
	std::vector<T> unifRandSelectionVec(const std::vector<T> & vec, uint32_t amt,
			bool withReplacement) {
		std::vector<T> ret;
		std::vector<uint32_t> rSel;
		if (withReplacement) {
			rSel = unifRandVector<uint32_t>(0, vec.size(), amt);
		} else {
			if (amt > vec.size()) {
				std::stringstream ss;
				ss << "Error in unifRandSelectionVec, requesting"
						" more than is in vec but said without replacement" << std::endl;
				throw std::runtime_error { ss.str() };
			} else {
				std::vector<uint32_t> rSelPos(vec.size());
				iota<uint32_t>(rSelPos, 0);
				shuffle(rSelPos, mtGen_);
				rSel = getSubVector(rSelPos, 0, amt);
			}
		}
		for (const auto & pos : rSel) {
			ret.emplace_back(vec[pos]);
		}
		return ret;
	}

	/**@brief seed mtGen_ with a new seed from random_device
	 *
	 */
	void seed() {
		std::random_device rd;
		seedNum(rd());
	}
	/**@brief seed mtGen_ with the given seed
	 *
	 * @param givenSeed
	 */
	void seedNum(uint64_t givenSeed) {
		currentSeed_ = givenSeed;
		mtGen_.seed(givenSeed);
	}
};

}  // namespace njh
