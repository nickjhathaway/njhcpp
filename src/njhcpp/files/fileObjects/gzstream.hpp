#pragma once
// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : gzstream.h
// Revision      : $Revision: 1.5 $
// Revision_date : $Date: 2002/04/26 23:30:15 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Standard streambuf implementation following Nicolai Josuttis, "The 
// Standard C++ Library".
// ============================================================================

// standard C++ with new header file names and std:: namespace
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <boost/filesystem.hpp>

namespace njh {
namespace bfs = boost::filesystem;

namespace GZSTREAM {

/**@brief The zlib compression level gzip output is written with unless set otherwise,
 * 3 writes ~3.5x faster than zlib's default of 6 for files ~25% bigger (see setCompressionLevel)
 */
constexpr int defaultCompressionLevel = 3;

// ----------------------------------------------------------------------------
// Internal classes to implement gzstream. See below for user classes.
// ----------------------------------------------------------------------------

class gzstreambuf: public std::streambuf {
private:
	//zlib keeps its own 128kb buffer (see gzbuffer in open) so this only sets how often gzread/gzwrite are called, 64kb keeps that rare
	static const int bufferSize = 64 * 1024;

	gzFile file_ = nullptr;               // file handle for compressed file
	std::vector<char> buffer_ = std::vector<char>(bufferSize); // data buffer
	char opened_;             // open/close state of stream
	int mode_ = -1;               // I/O mode
	int compressionLevel_ = defaultCompressionLevel; //zlib compression level 0-9 when writing, -1 for zlib's own default (6)
	uint64_t uncompressedSinceMemberEnd_ = 0; //bytes given to zlib since open or the last endMember()

	int flush_buffer() {
		// Separate the writing of the buffer from overflow() and
		// sync() operation.
		int w = pptr() - pbase();
		if (gzwrite(file_, pbase(), w) != w) {
			return EOF;
		}
		uncompressedSinceMemberEnd_ += w;
		pbump(-w);
		return w;
	}
public:
	gzstreambuf() :
			opened_(0) {
		setp(buffer_.data(), buffer_.data() + (bufferSize - 1));
		setg(buffer_.data() + 4,     // beginning of putback area
		buffer_.data() + 4,     // read position
		buffer_.data() + 4);    // end position
		// ASSERT: both input & output capabilities will not be used together
	}

	/**@brief Set the compression level used when the stream is opened for writing, has to be set before open
	 *
	 * @param compressionLevel 0-9 (1 fastest, 9 smallest) or -1 for zlib's own default (6)
	 */
	void setCompressionLevel(int compressionLevel) {
		if (compressionLevel < -1 || compressionLevel > 9) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error compression level should be between 0 and 9 or -1 for the default, not " << compressionLevel << "\n";
			throw std::runtime_error { ss.str() };
		}
		compressionLevel_ = compressionLevel;
	}

	/**@brief Finish the current gzip member, anything written after starts a new member in the same file and the file still reads as one gzip stream
	 *
	 * @return whether anything was written to the member just finished, nothing is done if not
	 */
	bool endMember() {
		if (!opened_ || !(mode_ & std::ios::out)) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, stream isn't open for writing" << "\n";
			throw std::runtime_error { ss.str() };
		}
		if (-1 == sync()) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in writing buffer" << "\n";
			throw std::runtime_error { ss.str() };
		}
		if (0 == uncompressedSinceMemberEnd_) {
			return false;
		}
		auto status = gzflush(file_, Z_FINISH);
		if (Z_OK != status) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in gzflush, status " << status << "\n";
			throw std::runtime_error { ss.str() };
		}
		uncompressedSinceMemberEnd_ = 0;
		return true;
	}

	/**@brief The number of compressed bytes written to the file so far, only exact right after open or endMember() since zlib holds compressed data until then
	 *
	 * @return the compressed offset
	 */
	uint64_t compressedOffset() const {
		return static_cast<uint64_t>(gzoffset(file_));
	}
	int is_open() {
		return opened_;
	}

	~gzstreambuf() {
		close();
	}
	gzstreambuf* open(const char* name, int open_mode) {
		if (is_open()) {
			return (gzstreambuf*) 0;
		}
		mode_ = open_mode;
		// no append nor read/write mode
//		if ((mode_ & std::ios::ate) || (mode_ & std::ios::app)
//				|| ((mode_ & std::ios::in) && (mode_ & std::ios::out))) {
//			return (gzstreambuf*) 0;
//		}

		if ((mode_ & std::ios::app)
				|| ((mode_ & std::ios::in) && (mode_ & std::ios::out))) {
			return (gzstreambuf*) 0;
		}

		char fmode[10];
		char* fmodeptr = fmode;
		if (mode_ & std::ios::in) {
			*fmodeptr++ = 'r';
		} else if (mode_ & std::ios::out) {
			*fmodeptr++ = 'w';
		} else if(mode_ & std::ios::ate){
			*fmodeptr++ = 'a';
		}
		*fmodeptr++ = 'b';
		if ((mode_ & std::ios::out) && -1 != compressionLevel_) {
			*fmodeptr++ = static_cast<char>('0' + compressionLevel_);
		}
		*fmodeptr = '\0';
		bool alreadyExists = boost::filesystem::exists(name);
		file_ = gzopen(name, fmode);
		if(nullptr == file_){
			std::stringstream ss;

			ss << __PRETTY_FUNCTION__ << ", error " << " in opening " << name << " in mode: " << std::string(fmode) << ", exists: " << (alreadyExists? "true" : "false")<< "\n";
			throw std::runtime_error{ss.str()};
		}
#if ZLIB_VERNUM >= 0x1280
		//if you couldn't set the buffer, throw
		if (gzbuffer(file_, 128 * 1024)) {
			throw std::runtime_error { std::string(__PRETTY_FUNCTION__)
					+ ":couldn't set gz buffer, for " + std::string(name) + " in mode " + std::string(fmode) };
		}
#endif
		if (file_ == 0) {
			return (gzstreambuf*) 0;
		}
		opened_ = 1;
		uncompressedSinceMemberEnd_ = 0;
		return this;
	}

	gzstreambuf * close() {
		if (is_open()) {
			sync();
			opened_ = 0;
			if (gzclose(file_) == Z_OK) {
				return this;
			}
		}
		return (gzstreambuf*) 0;
	}

	virtual int underflow() { // used for input buffer only
		if (gptr() && (gptr() < egptr())) {
			return *reinterpret_cast<unsigned char *>(gptr());
		}
		if (!(mode_ & std::ios::in) || !opened_) {
			return EOF;
		}
		// Josuttis' implementation of inbuf
		int n_putback = gptr() - eback();
		if (n_putback > 4) {
			n_putback = 4;
		}
		memmove(buffer_.data() + (4 - n_putback), gptr() - n_putback, n_putback);

		int num = gzread(file_, buffer_.data() + 4, bufferSize - 4);
		if (num <= 0) { // ERROR or EOF
			return EOF;
		}
		// reset buffer pointers
		setg(buffer_.data() + (4 - n_putback),   // beginning of putback area
		buffer_.data() + 4,                 // read position
		buffer_.data() + 4 + num);          // end of buffer

		// return next character
		return *reinterpret_cast<unsigned char *>(gptr());
	}

	virtual int overflow(int c = EOF) { // used for output buffer only
		if (!opened_ || (!(mode_ & std::ios::out) && !(mode_ & std::ios::ate)) ) {
			return EOF;
		}
		if (c != EOF) {
			*pptr() = c;
			pbump(1);
		}
		if (flush_buffer() == EOF) {
			return EOF;
		}
		return c;
	}

	virtual int sync() {
		// Changed to use flush_buffer() instead of overflow( EOF)
		// which caused improper behavior with std::endl and flush(),
		// bug reported by Vincent Ricard.
		if (pptr() && pptr() > pbase()) {
			if (flush_buffer() == EOF) {
				return -1;
			}
		}
		return 0;
	}
};

class gzstreambase: virtual public std::ios {
protected:
	gzstreambuf buf;
public:

	gzstreambase() {
		init(&buf);
	}
	gzstreambase(const char* name, int mode) {
		init(&buf);
		open(name, mode);
	}
	~gzstreambase() {
		buf.close();
	}
	void open(const char* name, int open_mode) {
		if (!buf.open(name, open_mode)) {
			clear(rdstate() | std::ios::badbit);
		}
	}

	void close() {
		if (buf.is_open()) {
			if (!buf.close()) {
				clear(rdstate() | std::ios::badbit);
			}
		}
	}
	gzstreambuf* rdbuf() {
		return &buf;
	}
};

// ----------------------------------------------------------------------------
// User classes. Use igzstream and ogzstream analogously to ifstream and
// ofstream respectively. They read and write files based on the gz* 
// function interface of the zlib. Files are compatible with gzip compression.
// ----------------------------------------------------------------------------

class igzstream: public gzstreambase, public std::istream {
public:
	igzstream() :
			std::istream(&buf) {
	}
	igzstream(const bfs::path & name, int open_mode = std::ios::in) :
			gzstreambase(name.string().c_str(), open_mode), std::istream(&buf) {
	}
	gzstreambuf* rdbuf() {
		return gzstreambase::rdbuf();
	}
	void open(const bfs::path & name, int open_mode = std::ios::in) {
		gzstreambase::open(name.string().c_str(), open_mode);
	}
};

class ogzstream: public gzstreambase, public std::ostream {
public:
	ogzstream() :
			std::ostream(&buf) {
	}
	explicit ogzstream(const bfs::path & name, int mode = std::ios::out) :
			gzstreambase(name.string().c_str(), mode), std::ostream(&buf) {
	}
	gzstreambuf* rdbuf() {
		return gzstreambase::rdbuf();
	}
	void open(const bfs::path & name, int open_mode = std::ios::out) {
		gzstreambase::open(name.string().c_str(), open_mode);
	}
	/**@brief Open for writing with a compression level
	 *
	 * @param name the file
	 * @param compressionLevel 0-9 (1 fastest, 9 smallest) or -1 for zlib's own default (6)
	 * @param open_mode the mode
	 */
	void open(const bfs::path & name, int compressionLevel, int open_mode) {
		rdbuf()->setCompressionLevel(compressionLevel);
		gzstreambase::open(name.string().c_str(), open_mode);
	}
};

}  // namespace GZSTREAM
}  // namespace njh

