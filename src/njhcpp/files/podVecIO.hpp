#pragma once
/*

 * podVecIO.hpp
 *
 *  Created on: Apr 13, 2016
 *      Author: nick
 */

#include "njhcpp/files/fileUtilities.hpp"
#include <zlib.h>
#include "njhcpp/utils/typeUtils.hpp" //njh::TypeName::get

namespace njh {
namespace files {

/**@brief Write out a vector as a chunk of data to compressed binary file
 *
 * @param fnp The file to write to, will overwrite it if it already exits
 * @param d The vector to write
 */
template<typename T>
void writePODvectorGz(bfs::path fnp, const std::vector<T> d) {

	if (bfs::exists(fnp)) {
		bfs::remove(fnp);
	}
	njh::files::touch(fnp);
	uint64_t numBytes = d.size() * sizeof(T);
	//bfs::resize_file(fnp, numBytes);
	auto gzFileOut = gzopen(njh::appendAsNeededRet(fnp.string(), ".gz").c_str(), "w");
	/** @todo figure way to test if the file opened
	if (!()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}*/
	gzwrite(gzFileOut, d.data(), numBytes);
	gzclose(gzFileOut);

	//std::cout << "wrote " << fnp << " (" << d.size() << " elements)" << std::endl;
}

/**@brief Read a chunk of data from a compressed binary file, most likely written by njh::files::writePODvector
 *
 * @param fnp A filename to read the data from
 * @return The data from the file back as a vector
 */
template<typename T>
std::vector<T> readPODvectorGz(bfs::path fnp) {
	auto gzInFile = gzopen(fnp.string().c_str(), "r");
	auto pos = gztell(gzInFile);
	if (EOF == pos) {
		throw njh::err::Exception(njh::err::F() << __PRETTY_FUNCTION__ << ": error in opening " << fnp);
	}
	std::vector<T> ret;
	gzbuffer(gzInFile, 128 * 1024);
	uint32_t bufferSize = 10000;
	auto bufferBytes = sizeof(T) * bufferSize;
	std::vector<T> temp(bufferSize);
	while(!gzeof(gzInFile)){

		int32_t bytes_read = gzread(gzInFile, temp.data(), bufferBytes);
		if(0 != bytes_read % sizeof(T)){
			std::stringstream ss;
			ss << "Error in: " << __PRETTY_FUNCTION__
					<< " read in " << bytes_read
					<< " bytes which is not divisible by the size of " << njh::TypeName::get<T>()
					<< ", " << sizeof(T) << std::endl;
			throw std::runtime_error{ss.str()};
		}
		uint32_t elements_read = bytes_read/sizeof(T);
		if(0 != elements_read){
			ret.insert(ret.end(), temp.begin(), temp.begin() + elements_read);
		}
	}
	gzclose(gzInFile);
	return ret;
}


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
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}
	auto* cstr = reinterpret_cast<const char*>(d.data());
	out.write(cstr, numBytes);
	out.close();

	//std::cout << "wrote " << fnp << " (" << d.size() << " elements)" << std::endl;
}

/**@brief Read a chunk of data, most likely written by njh::files::writePODvector
 *
 * @param fnp A filename to read the data from
 * @return The data from the file back as a vector
 */
template<typename T>
std::vector<T> readPODvector(bfs::path fnp) {
	uint64_t numBytes = bfs::file_size(fnp);
	if (numBytes % sizeof(T) != 0) {
		throw njh::err::Exception(
				njh::err::F() << "wrong type for reading file " << fnp);
	}
	uint64_t numElements = numBytes / sizeof(T);

	std::ifstream in(fnp.string(), std::ios::binary | std::ios::in);
	if (!in.is_open()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}

	std::vector<T> d(numElements);
	in.read(reinterpret_cast<char*>(d.data()), numBytes);
	in.close();

	return d;
}


/**@brief write out a matrix of most likely number as binary format
 *
 * this function does not safety checks to ensure that all numbers are the same size
 * @param fnp the filename
 * @param mat the matrix to write
 */
template<typename T>
void writePODmatrixNocheck(bfs::path fnp, const std::vector<std::vector<T>> mat) {

	if (bfs::exists(fnp)) {
		bfs::remove(fnp);
	}
	njh::files::touch(fnp);
	uint64_t numBytes = mat.size() * mat.size() * sizeof(T);
	bfs::resize_file(fnp, numBytes);

	std::ofstream out(fnp.string(), std::ios::binary | std::ios::out);
	if (!out.is_open()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}
	for(const auto & row : mat){
		auto* cstr = reinterpret_cast<const char*>(row.data());
		out.write(cstr, numBytes);
	}
	out.close();
}

/**@brief write out a matrix of most likely number as binary format
 * This function checks for same size rows and then call the no check njh::writePODmatrixNocheck
 * @param fnp the filename
 * @param mat the matrix to write
 */
template<typename T>
void writePODmatrix(bfs::path fnp, const std::vector<std::vector<T>> mat) {
	if(mat.empty()){
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ": mat is empty";;
		throw std::runtime_error{ss.str()};
	}
	const size_t size = mat.front().size();
	for(const auto & row : mat){
		if(size != row.size()){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ": Not all rows in mat are the same size: " << size << std::endl;
			ss << row.size() << std::endl;
			throw std::runtime_error{ss.str()};
		}
	}
	writePODmatrixNocheck(fnp, mat);
}


/**@brief Write in a matrix of most likely numbers from a binary file
 *
 * @param fnp the file to read from
 * @param nCol the number of columns in the matrix
 * @return The read in matrix
 */
template<typename T>
std::vector<std::vector<T>> readPODmatrix(bfs::path fnp, uint32_t nCol) {
	uint64_t numBytes = bfs::file_size(fnp);
	if (numBytes % sizeof(T) != 0) {
		throw njh::err::Exception(
				njh::err::F() << __PRETTY_FUNCTION__ << ": wrong type for reading file, sizes don't make sense for file: " << fnp);
	}
	uint64_t numOfAllElements = numBytes / sizeof(T);
	if(0 != numOfAllElements % (nCol)){
		throw njh::err::Exception(
						njh::err::F() << __PRETTY_FUNCTION__ << ": number of columns, " << nCol << ", doesn't make sense with file size for file: " << fnp);
	}
	uint64_t numOfRowElements = numOfAllElements / nCol;
	uint64_t colBytes = sizeof(T) * nCol;

	std::ifstream in(fnp.string(), std::ios::binary | std::ios::in);
	if (!in.is_open()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}

	std::vector<std::vector<T>> mat = std::vector<std::vector<T>>(numOfRowElements, std::vector<T>(nCol));
	for(uint32_t row = 0; row <= numOfRowElements; ++row){
		in.read(reinterpret_cast<char*>(mat[row].data()), colBytes);
	}
	in.close();
	return mat;
}

/**@brief writes out a distance matrix, which only has half the matrix filled out, first vector can be empty to represent the empty top corner
 * Each row should be increasing in size by 1, this function does not check
 *
 * @param fnp the file to write to, will ovewrite
 * @param mat the matrix to write
 */
template<typename T>
void writePODDistMatNocheck(bfs::path fnp, const std::vector<std::vector<T>> mat) {

	if (bfs::exists(fnp)) {
		bfs::remove(fnp);
	}
	njh::files::touch(fnp);
	uint64_t numOfOrigElement = mat.size();
	if(!mat.front().empty()){
		numOfOrigElement = mat.size() + 1;
	}
	uint32_t numOfElements = ((numOfOrigElement - 1) * numOfOrigElement)/2;
	uint64_t numBytes = numOfElements * sizeof(T);
	bfs::resize_file(fnp, numBytes);

	std::ofstream out(fnp.string(), std::ios::binary | std::ios::out);
	if (!out.is_open()) {
		throw njh::err::Exception(njh::err::F() << __PRETTY_FUNCTION__ << ": could not open file " << fnp);
	}
	for(const auto & row : mat){
		if(0 == row.size()){
			continue;
		}
		auto* cstr = reinterpret_cast<const char*>(row.data());
		out.write(cstr, sizeof(T) * row.size());
	}
	out.close();
}

/**@brief writes out a distance matrix, which only has half the matrix filled out, first vector can be empty to represent the empty top corner
 * Each row should be increasing in size by 1, this function will check first and then call the no check function njh::writePODDistMatNocheck
 *
 * @param fnp the file to write to, will ovewrite
 * @param mat the matrix to write
 */
template<typename T>
void writePODDistMat(bfs::path fnp, const std::vector<std::vector<T>> mat) {
	if(mat.empty() || mat.size() < 2){
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ": mat is empty";;
		throw std::runtime_error{ss.str()};
	}
	uint32_t offSet = 0;
	if(!mat.front().empty()){
		offSet = 1;
	}
	for(const auto pos : iter::range<size_t>(offSet, mat.size())){
		if((pos + offSet) != mat[pos].size()){
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ": row: " << pos << ", should be size " << (pos + offSet) << " but is " << mat[pos].size();;
			throw std::runtime_error{ss.str()};
		}
	}
	writePODDistMatNocheck(fnp, mat);
}


/**@brief read in a distance matrix of likely numbers from a binary file, each row size should increase by 1 making the matrix only half full
 *
 * @param fnp
 * @param numOfOrigElement
 * @return
 */
template<typename T>
std::vector<std::vector<T>> readPODDistMatrix(bfs::path fnp, uint32_t numOfOrigElement) {
	uint64_t numBytes = bfs::file_size(fnp);
	if (numBytes % sizeof(T) != 0) {
		throw njh::err::Exception(
				njh::err::F() << __PRETTY_FUNCTION__ << ": wrong type for reading file, sizes don't make sense for file: " << fnp);
	}
	uint32_t numOfElements = ((numOfOrigElement - 1) * numOfOrigElement)/2;
	uint64_t expectedNumBytes = numOfElements * sizeof(T);

	if (0 != numBytes % (expectedNumBytes)) {
		throw njh::err::Exception(
				njh::err::F() << __PRETTY_FUNCTION__ << ": number of orginal elements, "
						<< numOfOrigElement
						<< ", doesn't make sense with file size for file: " << fnp);
	}

	std::ifstream in(fnp.string(), std::ios::binary | std::ios::in);
	if (!in.is_open()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}

	std::vector<std::vector<T>> mat(numOfOrigElement);

	for(uint32_t row = 0; row <= numOfOrigElement; ++row){
		mat[row].resize(row);
		std::cout << mat[row].size() << std::endl;
		if(0 != row){
			in.read(reinterpret_cast<char*>(mat[row].data()), row * sizeof(T));
		}
	}
	in.close();
	return mat;
}

}  // namespace files

}  // namespace njh
