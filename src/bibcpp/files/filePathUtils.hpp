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
#include <sys/stat.h>

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
	std::string eStr = estd::to_string(e);
	if ("" == path) {
		path.append(eStr);
	} else {
		if (!endsWith(path, "/") && !beginsWith(eStr, "/")) {
			path.push_back('/');
		} else if (endsWith(path, "/") && beginsWith(eStr, "/")) {
			eStr.erase(eStr.begin());
		}
		path.append(eStr);
	}
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
	return bfs::path(ret);
}


/**@brief Joining parent path with child path with a unix dir separator if needed
 *
 * Modeled after python's os.path.join
 *
 * @param par A parent path
 * @param child A file under parent path
 * @return Return par plus a unix directory separator if neccesary plus child
 */
inline bfs::path join(const std::string & par, const std::string & child){
	return make_path(par, child);
}

/**@brief Join all strings in vector paths with a unix dir separator if needed
 *
 * @param paths Paths to join
 * @return The contents of paths concatenated with a unix dir separator if needed
 */
inline bfs::path join(const std::vector<std::string> & paths){
	if(paths.size() == 1){
		return paths.front();
	}
	if(paths.size() == 0){
		return "";
	}
	return make_path(paths);
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
inline std::string removeExtension(const bfs::path& filename) {
	return bfs::path(filename).replace_extension("").string();
}

/**@brief Wrapper boost filesystem to get the a string of the extension for a given filename without the .
 *
 * @param filename Filename to get the extension for
 * @return The filename's extension
 */
inline std::string getExtension(const bfs::path& filename) {
	auto ret = bfs::path(filename).extension().string();
	if(ret.length() > 1){
		if(ret[0] == '.'){
			ret = ret.substr(1);
		}
	}
	return ret;
}


/**@brief Wrapper boost filesystem to get the a string of the parent path for a given filename
 *
 * @param filename A the filename to look for a parent path
 * @return The parent path
 */
inline std::string getParentPath(const bfs::path& filename) {
	return bfs::path(filename).parent_path().string();
}

/**@brief Find a filename that doesn't exist by appending _NUM if outFilename exists until outFilename_NUM doesn't exist
 *
 * @param outFilename Filename to check for existance
 * @return A filename for a file that doesn't exist
 */
inline bfs::path findNonexitantFile(const bfs::path & outFilename){
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

/**@brief To simply append to the end of the file path name to further extend the name with something but not a new directory
 *
 * @param fnp The file name path to append to
 * @param app what to append
 * @return a boost::filesystem::path with the fnp append with app
 */
inline bfs::path nameAppend(const bfs::path & fnp, const std::string & app){
	return bfs::path(fnp.string() + app);
}

/**@brief To append to the end of the file path name but before the file extension
 *
 * @param fnp The file name path to append to
 * @param app what to append
 * @return a boost::filesystem::path with the fnp append with app before the extension on fnp
 */
inline bfs::path nameAppendBeforeExt(const bfs::path & fnp, const std::string & app){
	return bfs::path(bfs::path(fnp).replace_extension("").string() + app + fnp.extension().string());
}

/**@brief To append to the filename taking into account if it's a full path, so prepend just the file basename not the whole file
 *
 * @param fnp The file name path to prepend to
 * @param app what to prepend
 * @return a boost::filesystem::path with just the last filename in the filename path prepended with pre
 */
inline bfs::path prependFileBasename(const bfs::path & fnp, const std::string & pre){
	return bib::files::make_path(fnp.parent_path(), pre + fnp.filename().string());
}

/**@brief check for the existence of a file and throw an exception if it doesn't exits
 *
 * @param fnp the file to check for
 * @param funcName the name of the function that the check is done from
 */
inline void checkExistenceThrow(const bfs::path & fnp, const std::string & funcName) {
	if (!bfs::exists(fnp)) {
		std::stringstream ss;
		ss << "Error in : " << funcName << std::endl;
		ss << fnp << " needs to exist" << std::endl;
		throw std::runtime_error { ss.str() };
	}
}
/**@brief check for the existence of a file and throw an exception if it doesn't exits
 *
 * @param fnp the file to check for
 *
 */
inline void checkExistenceThrow(const bfs::path & fnp) {
	if (!bfs::exists(fnp)) {
		std::stringstream ss;
		ss << "Error, " << fnp << " needs to exist" << std::endl;
		throw std::runtime_error { ss.str() };
	}
}

/**@brief make a file executable plus read and write for user and group and read for everyone, will throw if fnp doesn't exist
 *
 * @param fnp the path the file
 */
inline void chmod775(const bfs::path & fnp){
	checkExistenceThrow(fnp, __PRETTY_FUNCTION__);
	//make file executable
	chmod(fnp.string().c_str(),
			S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IEXEC | S_IXGRP);

}



}  // namespace files
}  // namespace bib

