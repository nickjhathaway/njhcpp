#pragma once
/*
 * filePathUtils.hpp
 *
 *  Created on: Dec 29, 2015
 *      Author: nick
 */

#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp" //appendAsNeededRet()
#include "bibcpp/files/fileUtilities.hpp"


namespace estd {
template <>
inline std::string to_string(const boost::filesystem::path & path) {
  return path.string();
}

template <>
inline std::string to_string(const Json::Value & path) {
  return path.asString();
}

}  // namespace estd

namespace bib {
namespace files {
namespace bfs = boost::filesystem;

template<typename T>
void addAsPathToPath(std::string& path, const T& e) {
	bib::appendAsNeeded(path, "/");
	path.append(estd::to_string(e));
}

template<typename T>
void addAsPathToPath(std::string& path, const std::vector<T>& items) {
	for (const auto& e : items) {
		addAsPathToPath(path, e);
	}
}

template<typename T>
void pasteAsPathAdd(std::string & path, const T& last) {
	addAsPathToPath(path, last);
}

template<typename N, typename ... T>
void pasteAsPathAdd(std::string& path, const N& next, const T&... rest) {
	addAsPathToPath(path, next);
	pasteAsPathAdd(path, rest...);
}

/**@brief take several different types of objects and convert to a path
 *
 * @param items items to combine into a path name
 * @return a path from all the items given
 */
template<typename ... T>
bfs::path make_path(const T&... items) {
	std::string ret = "";
	pasteAsPathAdd(ret, items...);
	return bfs::path(ret.substr(1));
}


/**@brief Joining parent path with child path with a unix dir separator if needed
 *
 * Modeled after python's os.path.join
 *
 * @param par A parent path
 * @param child A file under parent path
 * @return Return par plus a unix directory separator if neccesary plus child
 */
inline std::string join(const std::string & par, const std::string & child){
	return appendAsNeededRet(par, "/") + child;
}

/**@brief Join all strings in vector paths with a unix dir separator if needed
 *
 * @param paths Paths to join
 * @return The contents of paths concatenated with a unix dir separator if needed
 */
inline std::string join(const std::vector<std::string> & paths){
	if(paths.size() == 1){
		return paths.front();
	}
	std::string ret = "";
	if(paths.size() == 0){
		return ret;
	}
	for(const auto & p : iter::range(paths.size() - 1)){
		ret += appendAsNeededRet(paths[p], "/");
	}
	return ret + paths.back();
}

/**@brief Return full path name even if the path doesn't actually exists
 *
 * @param path The path to get the full path name for
 * @return The full path with . or .. remove and by following symlinks
 */
inline bfs::path normalize(const boost::filesystem::path &path) {
	//from http://stackoverflow.com/questions/1746136/how-do-i-normalize-a-pathname-using-boostfilesystem
	//useful for when the file doesn't actually exist as calling canonical on it will make it fail
	bfs::path absPath = absolute(path);
	bfs::path::iterator it = absPath.begin();
	bfs::path result = *it++;

	// Get canonical version of the existing part
	for (; exists(result / *it) && it != absPath.end(); ++it) {
		result /= *it;
	}
	result = bfs::canonical(result);

	// For the rest remove ".." and "." in a path with no symlinks
	for (; it != absPath.end(); ++it) {
		// Just move back on ../
		if (*it == "..") {
			result = result.parent_path();
		}
		// Ignore "."
		else if (*it != ".") {
			// Just cat other path entries
			result /= *it;
		}
	}
	return result;
}

/**@brief convenience function to just get the string of the current path from a bfs path object of the current path
 *
 * @return A string of the current path
 */
inline std::string get_cwd() {
	return bfs::current_path().string();
}


/**@brief Get home directory, only works if $HOME set in environment
 *
 * @return The home directory if $HOME exists, blank otherwise
 */
inline std::string getHomeStr(){
	char * homeTest1 = ::getenv("HOME");
	if(homeTest1){
		return homeTest1;
	}else{
		return "";
	}
}


/**@brief Get a string of a filename with the extension removed, will preserve path name
 *
 * @param filename The filename to remove the extension
 * @return The filename with the extension removed
 */
inline std::string removeExtension(const std::string& filename) {
	return bfs::path(filename).replace_extension("").string();
}

/**@brief Wrapper boost filesystem to get the a string of the extension for a given filename
 *
 * @param filename Filename to get the extension for
 * @return The filename's extension
 */
inline std::string getExtension(const std::string& filename) {
	auto ret = bfs::path(filename).extension().string();
	if(ret.length() > 1){
		if(ret[0] == '.'){
			ret = ret.substr(1);
		}
	}
	return ret;
}

/**@brief Wrapper boost filesystem to get the a string of the filename without extension for a given filename
 *
 * @param filename The filename to get the filename for
 * @return The filename (without extention)
 */
inline std::string getFileName(const std::string& filename) {
	return bfs::basename(filename);
}

/**@brief Wrapper boost filesystem to get the a string of the parent path for a given filename
 *
 * @param filename A the filename to look for a parent path
 * @return The parent path
 */
inline std::string getPathName(const std::string& filename) {
	return bfs::path(filename).parent_path().string();
}

/**@brief Find a filename that doesn't exist by appending _NUM if outFilename exists until outFilename_NUM doesn't exist
 *
 * @param outFilename Filename to check for existance
 * @return A filename for a file that doesn't exist
 */
inline std::string findNonexitantFile(const std::string & outFilename){
	if(bfs::exists(outFilename)){
		std::string fileStub = removeExtension(outFilename);
		std::string extention = getExtension(outFilename);
		uint32_t fileCount = 1;
		std::string newOutFilename = fileStub + "_" + estd::to_string(fileCount) + "." + extention;
		while (bfs::exists(newOutFilename)){
			++fileCount;
			newOutFilename = fileStub + "_" + estd::to_string(fileCount) + "." + extention;
		}
		return newOutFilename;
	}else{
		return outFilename;
	}
}

}  // namespace files
}  // namespace bib

