#pragma once
/*
 * fileSystemUtils.hpp
 *
 *  Created on: Dec 29, 2015
 *      Author: nick
 */

#include <vector>
#include <string>
#include <map>
#include <iterator>
#include <fstream> //ofstream
#include <sys/stat.h> //chmod
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp" //appendAsNeededRet()
#include "bibcpp/utils/time/timeUtils.hpp" //getCurrentDate()

namespace bib {
namespace files {
namespace bfs = boost::filesystem;


// from http://boost.2283326.n4.nabble.com/filesystem-6521-Directory-listing-using-C-11-range-based-for-loops-td4570988.html
/**@brief Class to allow to iterator over a directory's content with c11 for loop semantics
 *
 */
class dir {
	bfs::path p_;

public:
	inline dir(bfs::path p) :
			p_(p) {
	}

	bfs::directory_iterator begin() {
		return bfs::directory_iterator(p_);
	}

	bfs::directory_iterator end() {
		return bfs::directory_iterator();
	}
};

/**@brief List the files in directory d
 *
 * @param d The directory to list the files contents of
 * @return Return the a vector of the paths of the directory d
 */
inline std::vector<bfs::path> filesInFolder(bfs::path d) {
	std::vector<bfs::path> ret;
	if (bfs::is_directory(d)) {
		for (const auto& e : dir(d)) {
			ret.emplace_back(e);
		}
	}

	return ret;
}

/**@brief Helper function for listing files recursively
 *
 * @param dirName The name of directory to search
 * @param recursive Whether the search should be recursive
 * @param files The map container in which to store results
 * @param currentLevel The current level of the search if recursive
 * @param levels The max level to search to, level 1 be just the current directory, 2 being the contents of directories of the current dir, etc.
 */
inline void listAllFilesHelper(const bfs::path & dirName, bool recursive,
		std::map<bfs::path, bool> & files,
		uint32_t currentLevel,
		uint32_t levels){

	bfs::directory_iterator end_iter;
	if(bfs::exists(dirName) && bfs::is_directory(dirName)){
	  for(const auto & dir_iter : dir(dirName)){
	  	bfs::path current = dir_iter.path();
	  	if(bfs::is_directory(dir_iter.path())){
	  		files[bfs::canonical(current)] = true;
	  		if(recursive && currentLevel <= levels){
	  			listAllFilesHelper(current, recursive, files,
	  					currentLevel + 1, levels);
	  		}
	  	}else{
	  		files[bfs::canonical(current)] = false;
	  	}
	  }
	}
}

/**@brief Function to list all the files of a directory with the option to search recursively and name filtering
 *
 * @param dirName the name of the directory to search
 * @param recursive Whether the search should be recursive
 * @param contains A vector of strings that the path names must contains to be returned
 * @param levels The maximum number of levels to search
 * @return A map of the directory paths with key being the file path and the value being a bool indicating if it is a directory or not
 */
inline std::map<bfs::path, bool> listAllFiles(const std::string & dirName,
		bool recursive, const std::vector<std::string>& contains,
		uint32_t levels = std::numeric_limits<uint32_t>::max()){
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if(!contains.empty()){
		std::map<bfs::path, bool> specificFiles;
		for(const auto & f : files){
			if(checkForSubStrs(f.first.string(), contains)){
				specificFiles.emplace(f);
			}
		}
		return specificFiles;
	}
	return files;
}

/**@brief List files in a directory with optional recursive search and checking for regex patterns
 *
 * @param dirName The directory to search
 * @param recursive Whether the search should be recursive
 * @param contains A series of regex patterns the file path name has to contain
 * @param levels The maximum number of levels to search (1 being the first directory)
 * @return A map of boost::filesystem paths with the value being a bool with true indicating it's a directory
 */
inline std::map<bfs::path, bool> listAllFiles(const std::string & dirName,
		bool recursive, const std::vector<std::regex>& contains,
		uint32_t levels = std::numeric_limits<uint32_t>::max()){
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if(!contains.empty()){
		std::map<bfs::path, bool> specificFiles;
		for(const auto & f : files){
			if(checkForPats(f.first.string(), contains)){
				specificFiles.emplace(f);
			}
		}
		return specificFiles;
	}
	return files;
}

/**@brief List files in a directory with optional recursive search and checking for regex patterns
 *
 * @param dirName The directory to search
 * @param recursive Whether the search should be recursive
 * @param contains A series of regex patterns the file path name has to contain
 * @param excludes A series of regex patterns the file path name has to not contain
 * @param levels The maximum number of levels to search (1 being the first directory)
 * @return A map of boost::filesystem paths with the value being a bool with true indicating it's a directory
 */
inline std::map<bfs::path, bool> listAllFiles(const std::string & dirName,
		bool recursive, const std::vector<std::regex>& contains,
		const std::vector<std::regex>& excludes,
		uint32_t levels = std::numeric_limits<uint32_t>::max()){
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if(!contains.empty() || !excludes.empty()){
		std::map<bfs::path, bool> specificFiles;
		for(const auto & f : files){
			if(checkForPats(f.first.string(), contains) &&
				 checkForPatsExclude(f.first.string(), excludes)){
				specificFiles.emplace(f);
			}
		}
		return specificFiles;
	}
	return files;
}


/**@brief Open a ofstream with filename and checking for file existence
 *
 * @param file the ofstream object to open
 * @param filename The name of the file to open
 * @param overWrite Whether the file should be overwritten if it already exists
 * @param append Whether the file should be appended if it already exists
 * @param exitOnFailure whether program should exit on failure to open the file
 * @todo probably should just remove exitOnFailure and throw an exception instead
 */
inline void openTextFile(std::ofstream& file, const std::string & filename,
                         bool overWrite, bool append, bool exitOnFailure) {

  if (bfs::exists(filename) && !overWrite) {
    if (append) {
      file.open(filename.data(), std::ios::app);
    } else {
    	std::stringstream ss;
      ss << filename << " already exists";
      if (exitOnFailure) {
        throw std::runtime_error{ss.str()};
      }else{
      	std::cout << ss.str() << std::endl;
      }
    }
  } else {
    file.open(filename.data());
    if (!file) {
    	std::stringstream ss;
    	ss << "Error in opening " << filename;
      if (exitOnFailure) {
        throw std::runtime_error{ss.str()};
      }else{
      	std::cout << ss.str() << std::endl;
      }
    } else {
      chmod(filename.c_str(),
            S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
  }
}

/**@brief Open a ofstream with filename and checking for file existence
 *
 * @param file the ofstream object to open
 * @param filename The name of the file to open
 * @param overWrite Whether the file should be overwritten if it already exists
 * @param append Whether the file should be appended if it already exists
 * @param exitOnFailure whether program should exit on failure to open the file
 * @todo probably should just remove exitOnFailure and throw an exception instead
 */
inline void openTextFile(std::ofstream& file, std::string filename, const std::string & extention,
                         bool overWrite, bool append, bool exitOnFailure) {
	appendAsNeeded(filename, extention);
	openTextFile(file, filename, overWrite, append, exitOnFailure);
}


/**@brief make directory with given permissions and check for existence
 *
 * @param parentDirectory The directory wihin which to create the new directory
 * @param newDirectory The name of the new directory to create
 * @param check The stat of the dir creation, anyting but 1 should be a failure
 * @param perms The permissions for the directory defaults to read,write,execute for user and group and read,execute for others
 * @return The name of the created directory
 */
inline std::string checkMakeDir(std::string parentDirectory,
                         const std::string &newDirectory,
                         int &check,
                         mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) {
  if (parentDirectory.back() != '/') {
    parentDirectory.append("/");
  }
  std::string newDirectoryName = parentDirectory +
  		replaceString(newDirectory, "TODAY", getCurrentDate()) +
       "/";
  int32_t directoryStatus =
      mkdir(newDirectoryName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  if (directoryStatus != 0) {
    // directory already exits
  } else {
    chmod(newDirectoryName.c_str(), perms);
  }
  check = directoryStatus;
  return newDirectoryName;
}
/**@brief Make a directory in the given parent directory with the given permissions
 *
 * @param parentDirectory The directory wihin which to create the new directory
 * @param newDirectory The name of the new directory to create
 * @param perms The permissions for the directory defaults to read,write,execute for user and group and read,execute for others
 * @return The name of the created directory
 */
inline std::string makeDir(std::string parentDirectory,
                          std::string newDirectory,
                          mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) {
  int32_t directoryStatus = 0;
  std::string newDirectoryName = checkMakeDir(parentDirectory, newDirectory,
  		directoryStatus, perms);
  if (directoryStatus != 0) {
  	std::stringstream ss;
    ss << "Error in making directory " << newDirectoryName << std::endl;
    throw std::runtime_error{ss.str()};
  }
  return newDirectoryName;
}

/**@brief Make several directories in the given parent directory with the given permissions
 *
 * @param parentDirectory The directory wihin which to create the new directory
 * @param newDirectories The names of the new directories to create
 * @param perms The permissions for the directory defaults to read,write,execute for user and group and read,execute for others
 * @return The names of the created directories
 */
inline std::vector<std::string> makeDir(std::string parentDirectory,
													std::vector<std::string> newDirectories,
                          mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) {
	std::vector<std::string> ret;
	for(const auto & d : newDirectories){
		ret.emplace_back(makeDir(parentDirectory, d, perms));
	}
	return ret;
}

/**@brief remove a non empty directory forcibly
 *
 * @param dirName name of directory to remove
 * @return whether or not the removal of the directry was successful
 */
inline bool rmDirForce(const std::string & dirName){
	if(bfs::is_directory(dirName)){
		auto files = filesInFolder(dirName);
		for(const auto & f : files){
			rmDirForce(f.string());
		}
	}
	return bfs::remove(dirName);
}


}  // namespace files
}  // namespace bib


