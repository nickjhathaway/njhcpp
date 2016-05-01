#pragma once
/*

 * podVecIO.hpp
 *
 *  Created on: Apr 13, 2016
 *      Author: nick
 */

#include "bibcpp/files/fileUtilities.hpp"


namespace bib {
namespace files {

/**@brief Write out a vector as a chunk of data
 *
 * @param fnp The file to write to, will overwrite it if it already exits
 * @param d The vector to write
 */
template<typename T>
void writePODvector(bfs::path fnp, const std::vector<T> d) {

	if (bfs::exists(fnp)) {
		bfs::remove(fnp);
	}
	touch(fnp);
	uint64_t numBytes = d.size() * sizeof(T);
	bfs::resize_file(fnp, numBytes);

	std::ofstream out(fnp.string(), std::ios::binary | std::ios::out);
	if (!out.is_open()) {
		throw bib::err::Exception(bib::err::F() << "could not open file " << fnp);
	}
	auto* cstr = reinterpret_cast<const char*>(d.data());
	out.write(cstr, numBytes);
	out.close();

	//std::cout << "wrote " << fnp << " (" << d.size() << " elements)" << std::endl;
}

/**@brief Read a chunk of data, most likely written by bib::files::writePODvector
 *
 * @param fnp A filename to read the data from
 * @return The data from the file back as a vector
 */
template<typename T>
std::vector<T> readPODvector(bfs::path fnp) {
	uint64_t numBytes = bfs::file_size(fnp);
	if (numBytes % sizeof(T) != 0) {
		throw bib::err::Exception(
				bib::err::F() << "wrong type for reading file " << fnp);
	}
	uint64_t numElements = numBytes / sizeof(T);

	std::ifstream in(fnp.string(), std::ios::binary | std::ios::in);
	if (!in.is_open()) {
		throw bib::err::Exception(bib::err::F() << "could not open file " << fnp);
	}

	std::vector<T> d(numElements);
	in.read(reinterpret_cast<char*>(d.data()), numBytes);
	in.close();

	return d;
}

}  // namespace files

}  // namespace bib
