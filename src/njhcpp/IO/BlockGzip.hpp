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

#include <algorithm>
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
	/**@brief one line of an index, a block's key and its member's byte offset and size in the gzip file
	 *
	 */
	struct Entry {
		std::string key;
		uint64_t offset;
		uint64_t size;
	};

	/**
	 * @param gzFnp the block gzip file, its index is read from <gzFnp>.idx
	 */
	explicit BlockGzipIndex(const bfs::path & gzFnp) :
			gzFnp_(gzFnp) {
		for (auto & entry : readIndex(gzFnp_)) {
			keys_.emplace_back(entry.key);
			blocks_[entry.key] = std::make_pair(entry.offset, entry.size);
		}
	}

	/**@brief Read the index of a block gzip file
	 *
	 * @param gzFnp the block gzip file, its index is read from <gzFnp>.idx
	 * @return the index's entries in the order the blocks are in the file
	 */
	static std::vector<Entry> readIndex(const bfs::path & gzFnp) {
		auto idxFnp = BlockGzipOutputStream::indexPath(gzFnp);
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
		std::vector<Entry> ret;
		while (std::getline(in, line)) {
			if (line.empty()) {
				continue;
			}
			auto firstTab = line.find('\t');
			auto secondTab = std::string::npos == firstTab ? std::string::npos : line.find('\t', firstTab + 1);
			if (std::string::npos == firstTab || std::string::npos == secondTab) {
				std::stringstream ss;
				ss << __PRETTY_FUNCTION__ << ", error, " << idxFnp << " line should have 3 columns: " << line << "\n";
				throw std::runtime_error { ss.str() };
			}
			ret.emplace_back(Entry { line.substr(0, firstTab),
				std::stoull(line.substr(firstTab + 1, secondTab - firstTab - 1)),
				std::stoull(line.substr(secondTab + 1)) });
		}
		return ret;
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

	/**@brief decompress one or more gzip members laid end to end, e.g. a run of unindexed members
	 *
	 * @param compressed the members' bytes, has to be complete members
	 * @return the text
	 */
	static std::string gzipDecompressMembers(const std::string & compressed) {
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
		while (true) {
			zs.next_out = reinterpret_cast<Bytef *>(buffer.data());
			zs.avail_out = buffer.size();
			status = inflate(&zs, Z_NO_FLUSH);
			ret.append(buffer.data(), buffer.size() - zs.avail_out);
			if (Z_STREAM_END == status) {
				if (0 == zs.avail_in) {
					break;
				}
				//another member follows
				if (Z_OK != inflateReset(&zs)) {
					status = Z_DATA_ERROR;
					break;
				}
			} else if (Z_OK != status) {
				break;
			}
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

/**@brief Join block gzip files (written by BlockGzipOutputStream) and their indexes into one, copying the compressed blocks' bytes without decompressing or recompressing them.
 *
 * The output's blocks are in input order, its index has each block's offset shifted to where it now is.
 * Text written outside of blocks is in unindexed members, those are decompressed (they are expected to be small) to check them:
 * the leading text before an input's first block is its header, all inputs with one have to have the same header and it's written once at the start of the output,
 * unindexed text between or after blocks is copied as is, unindexed members with no text are dropped.
 * Each input is checked against its index first (blocks in order, inside the file and starting with a gzip member, unindexed bytes are complete gzip members)
 * so nothing is written for a bad input.
 *
 * @param inputs the block gzip files to join, in order, each needs its <file>.idx
 * @param out the joined file, its index is written to <out>.idx
 * @param failOnDuplicateKeys if true throw when a key is in more than one block, if false keep the first block for a key and drop the others
 * @param overWrite whether to overwrite out and its index if they already exist
 */
inline void concatBlockGzip(const std::vector<bfs::path> & inputs, const bfs::path & out,
		bool failOnDuplicateKeys = true, bool overWrite = false) {
	auto throwError = [](const std::string & mess) {
		std::stringstream ss;
		ss << "concatBlockGzip, error, " << mess << "\n";
		throw std::runtime_error { ss.str() };
	};
	auto outIdx = BlockGzipOutputStream::indexPath(out);
	for (const auto & fnp : { out, outIdx }) {
		if (bfs::exists(fnp) && !overWrite) {
			throwError(fnp.string() + " already exists, use overWrite to overwrite");
		}
	}
	//a byte range of an input to copy, a block if key is set
	struct Segment {
		uint64_t offset;
		uint64_t size;
		bool isBlock;
		std::string key;
	};
	struct Input {
		bfs::path fnp;
		std::vector<Segment> segments;
	};
	auto readBytes = [&throwError](std::ifstream & in, const bfs::path & fnp, uint64_t offset, uint64_t size) {
		std::string ret(size, '\0');
		in.clear();
		in.seekg(offset);
		in.read(ret.data(), ret.size());
		if (static_cast<std::streamsize>(ret.size()) != in.gcount()) {
			throwError("couldn't read " + std::to_string(size) + " bytes at " + std::to_string(offset) + " from " + fnp.string());
		}
		return ret;
	};

	//check every input and work out what to copy before writing anything
	std::vector<Input> plan;
	std::unordered_map<std::string, bfs::path> keyFiles;
	std::string header;
	std::string headerBytes;
	bfs::path headerFnp;
	for (const auto & fnp : inputs) {
		if (!bfs::exists(fnp)) {
			throwError(fnp.string() + " doesn't exist");
		}
		for (const auto & outFnp : { out, outIdx }) {
			if (bfs::exists(outFnp) && bfs::equivalent(fnp, outFnp)) {
				throwError("input " + fnp.string() + " is also the output " + outFnp.string());
			}
		}
		auto entries = BlockGzipIndex::readIndex(fnp);
		uint64_t fileSize = bfs::file_size(fnp);
		std::ifstream in(fnp.string(), std::ios::binary);
		if (!in) {
			throwError("couldn't open " + fnp.string());
		}
		Input input { fnp, { } };
		std::unordered_set<std::string> fileKeys;
		bool leading = true;
		uint64_t cursor = 0;
		//unindexed bytes between cursor and end, has to be whole gzip members
		auto addGap = [&](uint64_t end) {
			if (end == cursor) {
				return;
			}
			auto text = BlockGzipIndex::gzipDecompressMembers(readBytes(in, fnp, cursor, end - cursor));
			if (!text.empty()) {
				if (leading) {
					if (headerFnp.empty()) {
						header = text;
						headerBytes = readBytes(in, fnp, cursor, end - cursor);
						headerFnp = fnp;
					} else if (text != header) {
						throwError("the header in " + fnp.string() + " doesn't match the header in " + headerFnp.string());
					}
				} else {
					input.segments.emplace_back(Segment { cursor, end - cursor, false, "" });
				}
			}
		};
		for (const auto & entry : entries) {
			if (entry.offset < cursor || entry.offset > fileSize || entry.size > fileSize - entry.offset) {
				throwError("block " + entry.key + " in " + BlockGzipOutputStream::indexPath(fnp).string() + " at " + std::to_string(entry.offset)
						+ " size " + std::to_string(entry.size) + " overlaps the previous block or is past the end of " + fnp.string()
						+ " (" + std::to_string(fileSize) + " bytes), the index doesn't match the file");
			}
			if (entry.size > 0) {
				auto magic = readBytes(in, fnp, entry.offset, std::min<uint64_t>(2, entry.size));
				if ("\x1f\x8b" != magic) {
					throwError("block " + entry.key + " in " + fnp.string() + " at " + std::to_string(entry.offset) + " isn't the start of a gzip member, the index doesn't match the file");
				}
			}
			if (!fileKeys.emplace(entry.key).second) {
				throwError("key " + entry.key + " is in " + BlockGzipOutputStream::indexPath(fnp).string() + " more than once");
			}
			addGap(entry.offset);
			leading = false;
			cursor = entry.offset + entry.size;
			auto prev = keyFiles.find(entry.key);
			if (keyFiles.end() != prev) {
				if (failOnDuplicateKeys) {
					throwError("key " + entry.key + " is in both " + prev->second.string() + " and " + fnp.string());
				}
				continue;
			}
			keyFiles.emplace(entry.key, fnp);
			input.segments.emplace_back(Segment { entry.offset, entry.size, true, entry.key });
		}
		addGap(fileSize);
		plan.emplace_back(std::move(input));
	}

	//copy
	try {
		std::ofstream outFile(out.string(), std::ios::binary | std::ios::trunc);
		if (!outFile) {
			throwError("couldn't open " + out.string());
		}
		std::vector<BlockGzipIndex::Entry> outEntries;
		uint64_t outOffset = 0;
		outFile.write(headerBytes.data(), headerBytes.size());
		outOffset += headerBytes.size();
		std::vector<char> buffer(1024 * 1024);
		for (const auto & input : plan) {
			std::ifstream in(input.fnp.string(), std::ios::binary);
			if (!in) {
				throwError("couldn't open " + input.fnp.string());
			}
			for (const auto & segment : input.segments) {
				if (segment.isBlock) {
					outEntries.emplace_back(BlockGzipIndex::Entry { segment.key, outOffset, segment.size });
				}
				in.clear();
				in.seekg(segment.offset);
				uint64_t left = segment.size;
				while (left > 0) {
					auto chunk = static_cast<std::streamsize>(std::min<uint64_t>(left, buffer.size()));
					in.read(buffer.data(), chunk);
					if (chunk != in.gcount()) {
						throwError("couldn't read " + std::to_string(segment.size) + " bytes at " + std::to_string(segment.offset) + " from " + input.fnp.string());
					}
					outFile.write(buffer.data(), chunk);
					left -= chunk;
				}
				outOffset += segment.size;
			}
		}
		outFile.close();
		if (!outFile) {
			throwError("couldn't write " + out.string());
		}
		std::ofstream indexOut(outIdx.string(), std::ios::trunc);
		if (!indexOut) {
			throwError("couldn't open " + outIdx.string());
		}
		indexOut << "key\toffset\tsize\n";
		for (const auto & entry : outEntries) {
			indexOut << entry.key << "\t" << entry.offset << "\t" << entry.size << "\n";
		}
		indexOut.close();
		if (!indexOut) {
			throwError("couldn't write " + outIdx.string());
		}
	} catch (...) {
		//don't leave a partial output that looks complete
		boost::system::error_code ec;
		bfs::remove(out, ec);
		bfs::remove(outIdx, ec);
		throw;
	}
}

}  // namespace njh
