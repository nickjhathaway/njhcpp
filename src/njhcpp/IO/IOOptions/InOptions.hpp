#pragma once
/*
 * InOptions.hpp
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
#include "njhcpp/files/fileObjects/gzstream.hpp" //njh::GZSTREAM

//#include "njhcpp/files.h"

namespace njh {

class InOptions {
public:


	InOptions() :
			inFilename_(""), inExtention_(""), inFormat_("") {

	}

	InOptions(const bfs::path & filename) :
			inFilename_(filename), inExtention_(bfs::extension(filename)) {
	}

	InOptions(const bfs::path & filename,
			const std::string & extention, const std::string & format) :
			inFilename_(filename), inExtention_(extention), inFormat_(format) {
	}

	explicit InOptions(const Json::Value & val){
		inFilename_ = val.get("inFilename_", "").asString();
		inExtention_ = val.get("inExtention_", "").asString();
		inFormat_ = val.get("inFormat_", "").asString();
	}

	bfs::path inFilename_;
	std::string inExtention_;
	std::string inFormat_;

	bool inExists() const{
		return bfs::exists(inFilename_);
	}




	Json::Value toJson() const{
		Json::Value ret;
		ret["inFilename_"] = njh::json::toJson(inFilename_);
		ret["inExtention_"] = njh::json::toJson(inExtention_);
		ret["inFormat_"] = njh::json::toJson(inFormat_);
		return ret;
	}


	void openGzFile(njh::GZSTREAM::igzstream & inFile) const{
		if(!inExists()){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error attempted to open " << inFilename_ << " when it doesn't exist " << "\n";
			throw std::runtime_error{ss.str()};
		}
		if(bfs::is_directory(inFilename_)){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error attempted to open " << inFilename_ << " when it's a directory " << "\n";
			throw std::runtime_error{ss.str()};
		}
		inFile.open(inFilename_);
		if(!inFile){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in opening " << inFilename_ << " for reading " << "\n";
			throw std::runtime_error{ss.str()};
		}
	}

	void openFile(std::ifstream & inFile) const{
		if(!inExists()){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error attempted to open " << inFilename_ << " when it doesn't exist " << "\n";
			throw std::runtime_error{ss.str()};
		}
		if(bfs::is_directory(inFilename_)){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error attempted to open " << inFilename_ << " when it's a directory " << "\n";
			throw std::runtime_error{ss.str()};
		}

		inFile.open(inFilename_.string());
		if(!inFile){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in opening " << inFilename_ << " for reading " << "\n";
			throw std::runtime_error{ss.str()};
		}
	}
	std::streambuf* determineInBuf(std::ifstream & inFile) const{
		if("" != inFilename_ && "STDIN" != inFilename_){
			openFile(inFile);
			return inFile.rdbuf();
		}else{
			return std::cin.rdbuf();
		}
	}

	std::streambuf* determineInBuf(njh::GZSTREAM::igzstream & inFileGz) const{
		if("" != inFilename_ && "STDIN" != inFilename_){
			openGzFile(inFileGz);
			return inFileGz.rdbuf();
		}else{
			return std::cin.rdbuf();
		}
	}



	std::streambuf* determineInBuf(std::ifstream & inFile,
			njh::GZSTREAM::igzstream & inFileGz) const {
		if ("" != inFilename_ && "STDIN" != inFilename_) {
			/**@todo use libmagic to determine input file type rather than based on file extension*/
			if (njh::endsWith(inFilename_.string(), ".gz")) {
				openGzFile(inFileGz);
				return inFileGz.rdbuf();
			} else {
				openFile(inFile);
				return inFile.rdbuf();
			}
		} else {
			return std::cin.rdbuf();
		}
	}
};



}  // namespace njh

