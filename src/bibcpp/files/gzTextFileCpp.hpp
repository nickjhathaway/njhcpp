#pragma once
/*
 * gzTextFileCpp.hpp
 *
 *  Created on: Dec 21, 2015
 *      Author: nick
 */

#include <zlib.h>
#include <string>
#include <array>
#include <cstdint>

namespace bib {
namespace files {

/**@brief Class to read a text file that's been gz zipped
 *
 */
template<size_t BUFFER = 1200>
class gzTextFileCpp {
	gzFile file_; /**< a gzFile object*/
	const static uint32_t bufferSize_ = BUFFER; /**< buffer size of the reading in from gz ziped file in chars*/
	const uint32_t byteBufferSize_ = bufferSize_ * sizeof(char); /**< the calculated buffer size in bytes*/
	std::array<char, bufferSize_> charData_; /**< the data array to hold current data*/
	std::string buf_; /**< the string buffer*/
	std::string filename_; /**< the filename to be read*/
	std::string terminator_; /**< the line terminator*/
public:
	/**@brief Construct with the filename and the terminator for the file
	 *
	 * @param filename The gz text filename
	 * @param terminator The terminator for the lines in the text file
	 */
	gzTextFileCpp(const std::string & filename, const std::string & terminator =
			"\n") :
			filename_(filename), terminator_(terminator) {
		file_ = gzopen(filename.c_str(), "rb");
		//if file isn't opened, throw
		if (!file_) {
			throw std::runtime_error { std::string(__PRETTY_FUNCTION__)
					+ ":error in opening, " + filename };
		}
#ifndef NO_GZBUFFER_FUNC
		//if you couldn't set the buffer, throw
		if (gzbuffer(file_, 128 * 1024)) {
			throw std::runtime_error { std::string(__PRETTY_FUNCTION__)
					+ ":couldn't set gz buffer" };
		}
#endif
	}


	/**@brief Read in the next chunk of data and decompress from the file
	 *
	 * @param chunk store the next block of data in this string
	 * @return Whether data larger than at least one char was read from the file
	 * @todo might want to throw or report if bytes were read but were less than a char
	 */
	bool getNextChunk(std::string & chunk) {
		//first clear the contents of std::string & chunk in case nothing is read
		chunk.clear();
		if (gzeof(file_)) {
			return false;
		}
		int bytes_read = gzread(file_, charData_.data(), byteBufferSize_);
		if (bytes_read >= sizeof(char)) {
			chunk = std::string(std::begin(charData_),
					std::begin(charData_) + bytes_read / sizeof(char));
			return true;
		}
		return false;
	}
	/**@brief getline on newline
	 *
	 * @param line store next line info in this string
	 * @return if a next line was read
	 */
	bool getline(std::string & line) {
		//first clear the contents of std::string & line in case nothing is read
		line.clear();
		if (buf_.empty() && gzeof(file_)) {
			return false;
		}
		//add to buffer until the end of the file or until the terminator is found
		std::string temp = "";
		while (buf_.find(terminator_) == std::string::npos && getNextChunk(temp)) {
			buf_.append(temp);
		}
		//if the buffer is not empty, trim from it the data up to the terminator
		if (!buf_.empty()) {
			//find terminator position and set line equal to the buffer from the beginning to the terminator
			//works for empty lines since .substr(0,0) returns an empty string
			//works for when the end of the file is reached and there is no terminator at the end
			//since .substr(0,std::string::npos) return the whole string
			auto termPos = buf_.find(terminator_);
			line = buf_.substr(0, termPos);
			if (termPos != std::string::npos && termPos + terminator_.size() < buf_.size()) {
				buf_ = buf_.substr(termPos + terminator_.size());
			} else {
				buf_ = "";
			}
			return true;
		}
		return false;
	}
	/**@brief A bool check on the underlying gzfile, should be used to see if file was opened
	 *
	 */
	explicit operator bool() const {
		return file_;
	}

	/**@brief Whether the buffer is empty and the gzfile is at the end of the file
	 *
	 * @return Whether there is anything else to be read
	 */
	bool done() {
		if("" == buf_ && !gzeof(file_)){
			std::string temp = "";
			if (getNextChunk(temp)) {
				buf_.append(temp);
			}
		}
		return "" == buf_ && gzeof(file_);
	}

	/**@brief when the file is de-constructed, close the underlying gzfile
	 *
	 */
	~gzTextFileCpp() {
		gzclose(file_);
	}

	/**@brief Look at the next character that can be read from the gz file
	 *
	 *
	 * @return
	 */
	int peek() {
		if (buf_.empty()) {
			std::string temp = "";
			if (getNextChunk(temp)) {
				buf_.append(temp);
			}
		}
		if ("" == buf_ && gzeof(file_)) {
			return std::ifstream::eofbit;
		}
		return buf_.front();
	}
};

}  // namespace files
}  // namespace bib


