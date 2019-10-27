#pragma once
/*
 * IoOptions.hpp
 *
 *  Created on: Feb 23, 2017
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

#include "njhcpp/utils.h"

#include "njhcpp/IO/IOOptions/InOptions.hpp"
#include "njhcpp/IO/IOOptions/OutOptions.hpp"

//#include "njhcpp/files.h"

namespace njh {


class IoOptions {
public:


	IoOptions() :
			in_(InOptions()), out_(OutOptions()) {

	}
	explicit IoOptions(InOptions inOpts) :
			in_(inOpts), out_(OutOptions()) {

	}
	explicit IoOptions(OutOptions outOpts) :
			in_(InOptions()), out_(outOpts) {

	}
	explicit IoOptions(InOptions inOpts, OutOptions outOpts) :
			in_(inOpts), out_(outOpts) {

	}

	explicit IoOptions(const Json::Value & val) :
			in_(val.get("in_", "")), out_(val.get("out_", "")) {

	}
	InOptions in_;
	OutOptions out_;





	void setInOptions(const bfs::path & filename,
			const std::string & extention, const std::string & format) {
		in_.inFilename_ = filename;
		in_.inExtention_ = extention;
		in_.inFormat_ = format;
	}

	void setOutOptions(const bfs::path & filename,
			const std::string & extention, const std::string & format) {
		out_.outFilename_ = filename;
		out_.outExtention_ = extention;
		out_.outFileFormat_ = format;
	}

	void setWritingOptions(bool append, bool overWriteFile,
			bool exitOnFailureToWrite) {
		out_.append_ = append;
		out_.overWriteFile_ = overWriteFile;
		out_.exitOnFailureToWrite_ = exitOnFailureToWrite;
	}

	Json::Value toJson() const {
		Json::Value ret;
		ret["class"] = json::toJson(getTypeName(*this));
		ret["in_"] = in_.toJson();
		ret["out_"] = out_.toJson();
		return ret;
	}

};

}  // namespace njh


