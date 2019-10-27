#pragma once
/*
 * OutputFile.hpp
 *
 *  Created on: Jul 3, 2017
 *      Author: nick
 */
// njhcpp - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of njhcpp.
//
// njhcpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// njhcpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with njhcpp.  If not, see <http://www.gnu.org/licenses/>.
//
#include "njhcpp/IO/IOOptions/InOptions.hpp"
#include <mutex>

//#include "njhcpp/files.h"

namespace njh {

class InputStream : public std::istream {
public:
	InputStream(const InOptions & inOpts) : std::istream(std::cin.rdbuf()),
			inOpts_(inOpts),
			inFileGz_(std::make_unique<njh::GZSTREAM::igzstream>()),
			inFile_(std::make_unique<std::ifstream>()) {

		rdbuf(inOpts_.determineInBuf(*inFile_, *inFileGz_));
	}
	const InOptions inOpts_;

	std::unique_ptr<njh::GZSTREAM::igzstream> inFileGz_;
	std::unique_ptr<std::ifstream> inFile_;

	std::mutex mut_;

};




} /* namespace njh */


