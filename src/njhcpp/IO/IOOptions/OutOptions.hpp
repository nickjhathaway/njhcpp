#pragma once
/*
 * OutOptions.hpp
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
#include "njhcpp/files.h"

namespace njh {


class OutOptions {
public:
	OutOptions() :
			outFilename_(""), outExtention_(""), outFileFormat_("") {

	}

	OutOptions(const bfs::path & filename) :
			outFilename_(filename), outExtention_(njh::files::bfs::extension(filename)) {

	}

	OutOptions(const bfs::path & filename,
			const std::string & extention) :
			outFilename_(filename), outExtention_(extention) {

	}

	OutOptions(const bfs::path & filename,
			const std::string & extention, const std::string & format) :
			outFilename_(filename), outExtention_(extention), outFileFormat_(format) {

	}

	OutOptions(const bfs::path & filename,
			const std::string & extention, const std::string & format, bool append,
			bool overWriteFile, bool exitOnFailureToWrite) :
			outFilename_(filename), outExtention_(extention), outFileFormat_(format), append_(
					append), overWriteFile_(overWriteFile), exitOnFailureToWrite_(
					exitOnFailureToWrite) {

	}

	explicit OutOptions(const Json::Value & val){
		outFilename_ = val.get("outFilename_", "").asString();
		outExtention_ = val.get("outExtention_", "").asString();
		outFileFormat_ = val.get("outFileFormat_", "").asString();
		overWriteFile_ = val.get("overWriteFile_", false).asBool();
		exitOnFailureToWrite_ = val.get("exitOnFailureToWrite_", false).asBool();
		append_ = val.get("append_", false).asBool();
	}

	bfs::path outFilename_;
	std::string outExtention_;
	std::string outFileFormat_;

	bool append_ = false;
	bool overWriteFile_ = false;
	bool exitOnFailureToWrite_ = true;
	bool binary_ = false;
	bfs::perms permissions_{bfs::owner_read | bfs::owner_write | bfs::group_read | bfs::group_write | bfs::others_read};





	std::shared_ptr<std::ofstream> openFile() const{
		auto out = std::make_shared<std::ofstream>();
		openFile(*out);
		return out;
	}

	std::shared_ptr<std::ofstream> openExecutableFile() const{
		auto out = std::make_shared<std::ofstream>();
		openExecutableFile(*out);
		return out;
	}

	void openGzFile(njh::GZSTREAM::ogzstream & outFileGz) const{
		if (bfs::exists(outName()) && !overWriteFile_) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << " error, "<< outName() << " already exists";
			throw std::runtime_error { ss.str() };
		} else {
			outFileGz.open(outName());
			if (!outFileGz) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error in opening " << outName();
				throw std::runtime_error { ss.str() };
			} else {
				bfs::permissions(outName(), permissions_);
			}
		}
	}

	void openBinaryGzFile(njh::GZSTREAM::ogzstream & outFileGz) const{
		if (bfs::exists(outName()) && !overWriteFile_) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << " error, "<< outName() << " already exists";
			throw std::runtime_error { ss.str() };
		} else {
			outFileGz.open(outName(), std::ios::out | std::ios::binary);
			if (!outFileGz) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error in opening " << outName();
				throw std::runtime_error { ss.str() };
			} else {
				bfs::permissions(outName(), permissions_);
			}
		}
	}

	void openFile(std::ofstream & outFile) const {
		if (bfs::exists(outName()) && !overWriteFile_) {
			if (append_) {
				outFile.open(outName().string(), std::ios::app);
			} else {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error, "<< outName() << " already exists";
				throw std::runtime_error { ss.str() };
			}
		} else {
			outFile.open(outName().string());
			if (!outFile) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error in opening " << outName();
				throw std::runtime_error { ss.str() };
			} else {
				bfs::permissions(outName(), permissions_);
			}
		}
	}

	void openBinaryFile(std::ofstream & outFile) const {
		if (bfs::exists(outName()) && !overWriteFile_) {
			if (append_) {
				outFile.open(outName().string(), std::ios::binary | std::ios::app);
			} else {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error, "<< outName() << " already exists";
				throw std::runtime_error { ss.str() };
			}
		} else {
			outFile.open(outName().string(), std::ios::binary);
			if (!outFile) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << " error in opening " << outName();
				throw std::runtime_error { ss.str() };
			} else {
				bfs::permissions(outName(), permissions_);
			}
		}
	}





	void openExecutableFile(std::ofstream & out) const {
		openFile(out);
		bfs::permissions(outName(), permissions_ | bfs::owner_exe | bfs::group_exe | bfs::others_exe);
	}

	Json::Value toJson() const{
		Json::Value ret;

		ret["outFilename_"] = njh::json::toJson(outFilename_);
		ret["outExtention_"] = njh::json::toJson(outExtention_);
		ret["outFileFormat_"] = njh::json::toJson(outFileFormat_);

		ret["binary_"] = njh::json::toJson(binary_);
		ret["append_"] = njh::json::toJson(append_);
		ret["overWriteFile_"] = njh::json::toJson(overWriteFile_);
		ret["exitOnFailureToWrite_"] = njh::json::toJson(exitOnFailureToWrite_);
		ret["permissions_"] = njh::json::toJson(njh::octToDec(permissions_));
		return ret;
	}

	bool outExists() const {
		return boost::filesystem::exists(outName());
	}

	void throwIfOutExistsNoOverWrite(const std::string & funcName) const {
		if(outExists() && !overWriteFile_ && !append_){
			std::stringstream ss;
			ss << funcName << ", error " << outName() << " already exists, set overWrite to true to over write" << "\n";
			throw std::runtime_error(ss.str());
		}
	}


	void transferOverwriteOpts(const OutOptions & otherOptions){
		overWriteFile_ = otherOptions.overWriteFile_;
		append_ = otherOptions.append_;
		exitOnFailureToWrite_ = otherOptions.exitOnFailureToWrite_;
	}


	bfs::path outName() const {
		return bfs::path(njh::appendAsNeededRet(outFilename_.string(), outExtention_));
	}

	/**@brief Will return the stream buffer for either the supplied std::ofstream if outFilename_ is not blank or std::cout
	 *
	 * used to construct a std::ostream object that will write to either the file if needed or to std::cout if outFilename_ is blank
	 *
	 * @param outFile the outFile that might be opened
	 * @return the stream buffer of either outFile or std::cout
	 */
	std::streambuf* determineOutBuf(std::ofstream & outFile) const {
		if ("" != outFilename_) {
			if (binary_) {
				openBinaryFile(outFile);
			} else {
				openFile(outFile);
			}
			return outFile.rdbuf();
		} else {
			return std::cout.rdbuf();
		}
	}

	/**@brief Will return stream buffer for either the supplied njh::GZSTREAM::ogzstream object if outFilename_ is not blank or std::cout
	 *
	 * @param outFileGz the outFile that might be opened
	 * @return the buffer of either the outfile or std::cout
	 */
	std::streambuf* determineOutBuf(
			njh::GZSTREAM::ogzstream & outFileGz) const {
		if ("" != outFilename_) {
			if (binary_) {
				openBinaryGzFile(outFileGz);
			} else {
				openGzFile(outFileGz);
			}
			return outFileGz.rdbuf();
		} else {
			return std::cout.rdbuf();
		}
	}

	/**@brief Will return the stream buffer for either the supplied std::ofstream or njh::GZSTREAM::ogzstream if outFilename_ is not blank  and based on the file extension or std::cout
	 *
	 * @param outFile the regular out file that might be opened
	 * @param outFileGz the gz out file that might be opened
	 * @return the buffer or std::cout, outFile or outFileGz
	 */

	std::streambuf* determineOutBuf(std::ofstream & outFile,
			njh::GZSTREAM::ogzstream & outFileGz) const {
		/**@todo perhaps having a bool member for gz output rather than determine by file name ending */
		if ("" != outFilename_) {
			if (("" != outExtention_ && njh::endsWith(outExtention_, ".gz")) || ("" == outExtention_ && njh::endsWith(outFilename_.string(), ".gz"))) {
				if (binary_) {
					openBinaryGzFile(outFileGz);
				} else {
					openGzFile(outFileGz);
				}
				return outFileGz.rdbuf();
			} else {
				if (binary_) {
					openBinaryFile(outFile);
				} else {
					openFile(outFile);
				}
				return outFile.rdbuf();
			}
		} else {
			return std::cout.rdbuf();
		}
	}

};










}  // namespace njh






