#pragma once
//
// njhseq - A library for analyzing sequence data
// Copyright (C) 2012-2018 Nicholas Hathaway <nicholas.hathaway@umassmed.edu>,
//
// This file is part of njhseq.
//
// njhseq is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// njhseq is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with njhseq.  If not, see <http://www.gnu.org/licenses/>.
//
//
//  IOUtils.hpp
//
//  Created by Nicholas Hathaway on 9/14/13.
//

#include "njhcpp/utils.h"
#include "njhcpp/IO/IOOptions.h"
#include "njhcpp/IO/OutputStream.hpp"
#include "njhcpp/IO/InputStream.hpp"
namespace njh {



/**
 * @brief gzip a file
 * @param opts the in and out options
 */
inline void gzZipFile(const IoOptions & opts){
	if (opts.out_.outExists() && !opts.out_.overWriteFile_) {
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ", error, file " << opts.out_.outName()
				<< " already exists, use --overWrite to overwrite it" << "\n";
		throw std::runtime_error { ss.str() };
	}
//	njh::GZSTREAM::ogzstream outStream(opts.out_.outName());
//	outStream << njh::files::get_file_contents(opts.in_.inFilename_, false);
	//read in chunks so that the entire file doesn't have to be read in if it's very large
	/**@todo find an apprioprate chunkSize or */
	uint32_t chunkSize = 4096 * 10;
	njh::GZSTREAM::ogzstream outstream;
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

namespace files{

/**
 * @brief take a file and copy it's contents into another file, can be used to convert between zip and unzip etc rather than a simple copy
 * @param inOpts the in options of the file to copy from
 * @param outOpts the out options of the file to copy into
 */
inline void reWriteFile(const InOptions & inOpts, const OutOptions & outOpts){
	InputStream inModel(inOpts);
	OutputStream outModel(outOpts);
	outModel << inModel.rdbuf();
	outModel.flush();
}

/**
 * @brief take a file and copy it's contents into another file, can be used to convert between zip and unzip etc rather than a simple copy
 * @param opts the in and out options for the files to copy to and from
 */
inline void reWriteFile(const IoOptions & opts){
	reWriteFile(opts.in_, opts.out_);
}


} // namespace files
} // namespace njh
