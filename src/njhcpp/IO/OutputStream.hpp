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
#include "njhcpp/IO/IOOptions/OutOptions.hpp"
//#include "njhcpp/files.h"

namespace njh {

class OutputStream : public std::ostream{
public:
	OutputStream(const OutOptions & outOpts) : std::ostream(std::cout.rdbuf()),
			outOpts_(outOpts),
			outFileGz_(std::make_unique<njh::GZSTREAM::ogzstream>()),
			outFile_(std::make_unique<std::ofstream>()) {

		rdbuf(outOpts_.determineOutBuf(*outFile_, *outFileGz_));
	}
	const OutOptions outOpts_;
	std::unique_ptr<njh::GZSTREAM::ogzstream> outFileGz_;
	std::unique_ptr<std::ofstream> outFile_;

	std::mutex mut_;

	~OutputStream(){
		//first flush current buffer before closing files, not sure if this is actually needed
		flush();
		outFile_ = nullptr;
		outFileGz_ = nullptr;
	}
};




} /* namespace njh */


