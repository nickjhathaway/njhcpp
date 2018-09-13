#pragma once
/*
 * OutputFile.hpp
 *
 *  Created on: Jul 3, 2017
 *      Author: nick
 */
// bibcpp - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of bibcpp.
//
// bibcpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bibcpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bibcpp.  If not, see <http://www.gnu.org/licenses/>.
//
#include "bibcpp/IO/IOOptions/OutOptions.hpp"
#include "bibcpp/files.h"

namespace bib {

class OutputStream : public std::ostream{
public:
	OutputStream(const OutOptions & outOpts) : std::ostream(std::cout.rdbuf()),
			outOpts_(outOpts),
			outFileGz_(std::make_unique<bib::GZSTREAM::ogzstream>()),
			outFile_(std::make_unique<std::ofstream>()) {

		rdbuf(outOpts_.determineOutBuf(*outFile_, *outFileGz_));
	}
	const OutOptions outOpts_;
	std::unique_ptr<bib::GZSTREAM::ogzstream> outFileGz_;
	std::unique_ptr<std::ofstream> outFile_;

	~OutputStream(){
		//first flush current buffer before closing files, not sure if this is actually needed
		flush();
		outFile_ = nullptr;
		outFileGz_ = nullptr;
	}
};




} /* namespace bib */


