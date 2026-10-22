#pragma once
/*
 * BlockGzip.hpp
 *
 *  Created on: Oct 22, 2026
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
#include "njhcpp/files/fileObjects/gzstream.hpp"

#include <array>
#include <fstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace njh {

/**@brief Write a gzip file where each keyed block is its own gzip member, and an index of where each block is, so one block can be read without decompressing the rest.
 * The file reads like any other gzip file (zcat, gzread etc. read straight through the members).
 *
 * The index is written to <file>.idx, tab delimited with a header, columns key, offset, size (the block's member's byte offset and size in the gzip file).
 * Text written outside of a block (e.g. a header) is in its own unindexed member. Write with << between startBlock() and endBlock().
 */
class BlockGzipOutputStream: public GZSTREAM::ogzstream {
public:
	/**
	 * @param outFnp the gzip file to write
	 * @param overWrite whether to overwrite the file and its index if they already exist
	 * @param compressionLevel 0-9 (1 fastest, 9 smallest) or -1 for zlib's own default (6)
	 */
	BlockGzipOutputStream(const bfs::path & outFnp, bool overWrite, int compressionLevel = GZSTREAM::defaultCompressionLevel) :
			outFnp_(outFnp) {
		for (const auto & fnp : { outFnp_, indexPath(outFnp_) }) {
			if (bfs::exists(fnp) && !overWrite) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << ", error " << fnp << " already exists, use overWrite to overwrite" << "\n";
				throw std::runtime_error { ss.str() };
			}
		}
		open(outFnp_, compressionLevel, std::ios::out);
		if (!rdbuf()->is_open()) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in opening " << outFnp_ << "\n";
			throw std::runtime_error { ss.str() };
		}
	}

	~BlockGzipOutputStream() {
		try {
			closeBlocks();
		} catch (const std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
	}

	/**@brief Start a block, everything written until endBlock() is indexed under key
	 *
	 * @param key the key, each can only be used once
	 */
	void startBlock(const std::string & key) {
		if (inBlock_) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, can't start block " << key << " before ending block " << currentKey_ << "\n";
			throw std::runtime_error { ss.str() };
		}
		if (!keys_.emplace(key).second) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, already wrote a block for " << key << " in " << outFnp_ << "\n";
			throw std::runtime_error { ss.str() };
		}
		//anything written before the block goes in its own member
		flush();
		rdbuf()->endMember();
		currentKey_ = key;
		currentStart_ = rdbuf()->compressedOffset();
		inBlock_ = true;
	}

	/**@brief End the current block, an empty block is indexed with size 0
	 *
	 */
	void endBlock() {
		if (!inBlock_) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, no block was started" << "\n";
			throw std::runtime_error { ss.str() };
		}
		flush();
		rdbuf()->endMember();
		index_.emplace_back(currentKey_, currentStart_, rdbuf()->compressedOffset() - currentStart_);
		inBlock_ = false;
	}

	/**@brief Write a whole block
	 *
	 * @param key the key, each can only be used once
	 * @param text the block's text
	 */
	void writeBlock(const std::string & key, const std::string & text) {
		startBlock(key);
		*this << text;
		endBlock();
	}

	/**@brief End any open block, close the file and write the index, also done on destruction
	 *
	 */
	void closeBlocks() {
		if (closed_) {
			return;
		}
		closed_ = true;
		if (inBlock_) {
			endBlock();
		}
		flush();
		close();
		std::ofstream indexOut(indexPath(outFnp_).string(), std::ios::trunc);
		if (!indexOut) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in opening " << indexPath(outFnp_) << "\n";
			throw std::runtime_error { ss.str() };
		}
		indexOut << "key\toffset\tsize\n";
		for (const auto & block : index_) {
			indexOut << std::get<0>(block) << "\t" << std::get<1>(block) << "\t" << std::get<2>(block) << "\n";
		}
	}

	/**@brief the index file for a block gzip file
	 *
	 * @param gzFnp the block gzip file
	 * @return the index's path
	 */
	static bfs::path indexPath(const bfs::path & gzFnp) {
		return bfs::path(gzFnp.string() + ".idx");
	}

private:
	bfs::path outFnp_;
	std::vector<std::tuple<std::string, uint64_t, uint64_t>> index_;
	std::unordered_set<std::string> keys_;
	std::string currentKey_;
	uint64_t currentStart_ { 0 };
	bool inBlock_ { false };
	bool closed_ { false };
};

/**@brief Read blocks of a file written by BlockGzipOutputStream using its index, read only after construction so it can be shared between threads
 *
 */
class BlockGzipIndex {
public:
	/**
	 * @param gzFnp the block gzip file, its index is read from <gzFnp>.idx
	 */
	explicit BlockGzipIndex(const bfs::path & gzFnp) :
			gzFnp_(gzFnp) {
		auto idxFnp = BlockGzipOutputStream::indexPath(gzFnp_);
		std::ifstream in(idxFnp.string());
		if (!in) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in opening " << idxFnp << "\n";
			throw std::runtime_error { ss.str() };
		}
		std::string line;
		std::getline(in, line);
		if ("key\toffset\tsize" != line) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, " << idxFnp << " should start with the header key, offset, size" << "\n";
			throw std::runtime_error { ss.str() };
		}
		while (std::getline(in, line)) {
			if (line.empty()) {
				continue;
			}
			auto firstTab = line.find('\t');
			auto secondTab = line.find('\t', firstTab + 1);
			if (std::string::npos == firstTab || std::string::npos == secondTab) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << ", error, " << idxFnp << " line should have 3 columns: " << line << "\n";
				throw std::runtime_error { ss.str() };
			}
			auto key = line.substr(0, firstTab);
			uint64_t offset = std::stoull(line.substr(firstTab + 1, secondTab - firstTab - 1));
			uint64_t size = std::stoull(line.substr(secondTab + 1));
			keys_.emplace_back(key);
			blocks_[key] = std::make_pair(offset, size);
		}
	}

	bool has(const std::string & key) const {
		return blocks_.end() != blocks_.find(key);
	}

	/**@brief the keys in the order the blocks are in the file
	 *
	 * @return the keys
	 */
	const std::vector<std::string> & keys() const {
		return keys_;
	}

	/**@brief Read and decompress one block
	 *
	 * @param key the block's key
	 * @return the block's text
	 */
	std::string readBlock(const std::string & key) const {
		auto search = blocks_.find(key);
		if (blocks_.end() == search) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, no block for " << key << " in " << BlockGzipOutputStream::indexPath(gzFnp_) << "\n";
			throw std::runtime_error { ss.str() };
		}
		if (0 == search->second.second) {
			return std::string { };
		}
		std::ifstream in(gzFnp_.string(), std::ios::binary);
		in.seekg(search->second.first);
		std::string compressed(search->second.second, '\0');
		in.read(compressed.data(), compressed.size());
		if (static_cast<std::streamsize>(compressed.size()) != in.gcount()) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, couldn't read block " << key << " from " << gzFnp_ << "\n";
			throw std::runtime_error { ss.str() };
		}
		return gzipDecompressMember(compressed);
	}

	/**@brief decompress a single gzip member
	 *
	 * @param compressed the member's bytes
	 * @return the text
	 */
	static std::string gzipDecompressMember(const std::string & compressed) {
		z_stream zs { };
		//15 + 16 to read a gzip header and trailer
		if (Z_OK != inflateInit2(&zs, 15 + 16)) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in inflateInit2" << "\n";
			throw std::runtime_error { ss.str() };
		}
		zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(compressed.data()));
		zs.avail_in = compressed.size();
		std::string ret;
		std::array<char, 64 * 1024> buffer;
		int status = Z_OK;
		while (Z_OK == status) {
			zs.next_out = reinterpret_cast<Bytef *>(buffer.data());
			zs.avail_out = buffer.size();
			status = inflate(&zs, Z_NO_FLUSH);
			ret.append(buffer.data(), buffer.size() - zs.avail_out);
		}
		inflateEnd(&zs);
		if (Z_STREAM_END != status) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error in inflate, status " << status << "\n";
			throw std::runtime_error { ss.str() };
		}
		return ret;
	}

private:
	bfs::path gzFnp_;
	std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> blocks_;
	std::vector<std::string> keys_;
};

}  // namespace njh
