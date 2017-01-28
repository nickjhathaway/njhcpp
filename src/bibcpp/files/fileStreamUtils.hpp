#pragma once
/*
 * fileStreamUtils.hpp
 *
 *  Created on: Dec 29, 2015
 *      Author: nick
 */

#include <string>
#include <iterator>
#include <fstream> //ofstream
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp" //appendAsNeededRet()
#include "bibcpp/files/fileSystemUtils.hpp"


namespace bib {
namespace files {
namespace bfs = boost::filesystem;


/**@brief Copy the contents of a file into a string
 *
 * @param fnp The name of the file
 * @param verbose Whether to print a statement about reading the file
 * @return A string containing the contents of the file
 */
inline static std::string get_file_contents(const bfs::path& fnp, bool verbose) {
	// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
	std::ifstream f(fnp.string(), std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		throw err::Exception(err::F() << __PRETTY_FUNCTION__ << " error :could not open file " << fnp);
	}
	if (verbose) {
		std::cout << "Reading file " << fnp << std::endl;
	}
	std::string ret;
	f.seekg(0, std::ios::end);
	ret.resize(f.tellg());
	f.seekg(0, std::ios::beg);
	f.read(&ret[0], ret.size());
	f.close();
	return ret;
}




/**@brief Get the streambuf of either an opened file or of std::cout if outFile is empty
 *
 * @param outFile The ofstream to open if needed
 * @param outFilename The name of the file, leave blank to get std::cout buffer
 * @param outFileExt The extention for the file
 * @param overWrite Whether to overwrite the file
 * @param append Whether to append to the file
 * @param exitOnFailure Whether if writing fails if the program should throw or just warn
 * @return A std::streambuf* or either an opened file or of std::cout
 */
inline std::streambuf* determineOutBuf(std::ofstream & outFile,
		const bfs::path & outFilename, const std::string outFileExt,
		bool overWrite, bool append, bool exitOnFailure) {
	if (outFilename != "") {
		openTextFile(outFile, outFilename, outFileExt, overWrite,
				append, exitOnFailure);
		return outFile.rdbuf();
	} else {
		return std::cout.rdbuf();
	}
}

/**@brief Cross platform get line to line CR and CRLF line endings
 *
 * @param __is The stream to read from
 * @param __str The string to store the info in
 * @return The stream read from
 */

inline std::istream&
crossPlatGetline(std::istream& __is, std::string& __str) {
	std::istream::sentry __sen(__is, true);
	if (__sen) {
		__str.clear();
		std::ios_base::iostate __err = std::ios_base::goodbit;
		std::streamsize __extr = 0;
		while (true) {
			std::istream::traits_type::int_type __i = __is.rdbuf()->sbumpc();
			if (std::istream::traits_type::eq_int_type(__i,
					std::istream::traits_type::eof())) {
				__err |= std::ios_base::eofbit;
				break;
			}
			++__extr;
			char __ch = std::istream::traits_type::to_char_type(__i);
			bool foundTerm = false;
			switch (__ch) {
			case '\n':
				foundTerm = true;
				break;
			case '\r':
				if (__is.rdbuf()->sgetc() == '\n') {
					__is.rdbuf()->sbumpc();
				}
				foundTerm = true;
				break;
			default:
				break;
			}
			if (foundTerm) {
				break;
			}
			__str.push_back(__ch);
			if (__str.size() == __str.max_size()) {
				__err |= std::ios_base::failbit;
				break;
			}
		}
		if (__extr == 0) {
			__err |= std::ios_base::failbit;
		}
		__is.setstate(__err);
	}
	return __is;
}


/**@brief Get the first line of a file, throws if there is an error in opening a file
 *
 * @param filename The name of the file
 * @return The first line of the file
 */
inline std::string getFirstLine(const bfs::path &filename) {
  std::string currentLine;
  std::ifstream textFile(filename.c_str());
  if (!textFile) {
  	throw std::runtime_error{"Error in opening " + filename.string()};
  }
  crossPlatGetline(textFile, currentLine);
  return currentLine;
}


/**@brief Get the last line of a file
 *
 * @param filename Name of the file to extract the last line from
 * @return A std::string object containing the last line of the file
 */
inline std::string getLastLine(const bfs::path & filename) {
	/*
	 Last line from the file, from http://www.programmersbook.com/page/7/Get-last-line-from-a-file-in-C/
	 */
	std::ifstream inputFile(filename.string(), std::ios_base::ate); //open file
	if(!inputFile){
		throw std::runtime_error{bib::bashCT::boldRed("Error in opening " + filename.string())};
	}
	std::string ret;
	int length = 0;
	char c = '\0';
	if (inputFile) {
		length = inputFile.tellg(); //Get file size
		// loop backward over the file
		for (int i = length - 2; i > 0; --i) {
			inputFile.seekg(i);
			c = inputFile.get();
			if (c == '\r' || c == '\n') //new line?
				break;
		}
		std::getline(inputFile, ret); //read last line
	}
	return ret;
}

/**@brief Get the next line ending with common line terminators without changing the stream's location or state
 *
 * @param is The stream to read from
 * @return The next line
 */
inline std::string peekLine(std::istream & is){
	std::string ret;
	auto currentPos = is.tellg();
	//auto state = is.rdstate();
	bib::files::crossPlatGetline(is, ret);
	is.seekg(currentPos);
	//is.setstate(state);
	return ret;
}

/**@brief Check if the next line the in file begins with the string
 *
 * @param is The input stream to read from
 * @param str The string to check for
 * @return Whether the next line begins with str
 */
inline bool nextLineBeginsWith(std::istream & is, const std::string & str){
	if(str.empty()){
		return false;
	}
	if(str.size() == 1){
		return str.front() == is.peek();
	}

	if(str.front() == is.peek()){
		auto nextLine = bib::files::peekLine(is);
		if(bib::beginsWith(nextLine, str)){
			return true;
		}
	}
	return false;
}

/**@brief Check if the next line the in file begins with the character
 *
 * @param is The input stream to read from
 * @param c The character to check for
 * @return Whether the next line begins with character c
 */
inline bool nextLineBeginsWith(std::istream & is, char c){
	return is.peek() == c;
}

/**@brief Count the number of lines in a file
 *
 * @param filename The filename to count
 * @return The number of lines in filename
 */
inline uint32_t countLines(const bfs::path & filename) {
	std::ifstream inFile(filename.string());
	if (!inFile) {
		std::stringstream ss;
		ss << "Error in opening " << filename << std::endl;
		throw std::runtime_error { ss.str() };
	}
	uint32_t ret = 0;
	std::string line;
	while (bib::files::crossPlatGetline(inFile, line)) {
		++ret;
	}
	return ret;
}


}  //namespace files
}  //namespace bib



