#pragma once
//
// bibseq - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of bibseq.
//
// bibseq is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bibseq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bibseq.  If not, see <http://www.gnu.org/licenses/>.
//
//
//  IOUtils.hpp
//
//  Created by Nicholas Hathaway on 9/14/13.
//

#include "bibcpp/utils.h"
#include "bibcpp/files.h"
#include "bibcpp/IO/IOOptions.h"
#include "bibcpp/IO/OutputStream.hpp"
#include "bibcpp/IO/InputStream.hpp"
namespace bib {

inline std::vector<std::string> getInputValues(const std::string & valuesStr,
		const std::string & delim) {
	std::vector<std::string> ret;
	if ("" == valuesStr) {
		return ret;
	}
	if (bfs::path(valuesStr).filename().string().length() <= 255
			&& bfs::exists(valuesStr)) {
		InputStream infile { bfs::path(valuesStr) };
		std::string line = "";
		while (bib::files::crossPlatGetline(infile, line)) {
			//skip empty or blank lines
			if(line.empty() || allWhiteSpaceStr(line)){
				continue;
			}
			ret.emplace_back(line);
		}
	} else {
		ret = tokenizeString(valuesStr, delim);
	}
	return ret;
}


inline void gzZipFile(const IoOptions & opts){
	if (opts.out_.outExists() && !opts.out_.overWriteFile_) {
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ", error, file " << opts.out_.outName()
				<< " already exists, use --overWrite to overwrite it" << "\n";
		throw std::runtime_error { ss.str() };
	}
//	bib::GZSTREAM::ogzstream outStream(opts.out_.outName());
//	outStream << bib::files::get_file_contents(opts.in_.inFilename_, false);
	//read in chunks so that the entire file doesn't have to be read in if it's very large
	/**@todo find an apprioprate chunkSize or */
	uint32_t chunkSize = 4096 * 10;
	bib::GZSTREAM::ogzstream outstream;
	opts.out_.openGzFile(outstream);
	std::ifstream infile(opts.in_.inFilename_.string(), std::ios::binary);
	std::vector<char> buffer(chunkSize);
	infile.read(buffer.data(), sizeof(char) * chunkSize);
	std::streamsize bytes = infile.gcount();

	while(bytes > 0){
		outstream.write(buffer.data(), bytes * sizeof(char));
		infile.read(buffer.data(), sizeof(char) * chunkSize);
		bytes = infile.gcount();
	}
}





}  // namespace bib
