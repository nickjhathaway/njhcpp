#pragma once

#include <vector>
#include <set>
#include <string>
#include <map>
#include <iterator>
#include <fstream> //ofstream
#include <sys/stat.h> //chmod
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp" //checkForSubStrs()
#include "bibcpp/utils/timeUtils.hpp" //getCurrentDate()
#include "bibcpp/debug/exception.hpp" //err
#include <chrono>
#include <regex>

namespace bib {
namespace files {

namespace bfs = boost::filesystem;
namespace sch = sch;
// from http://boost.2283326.n4.nabble.com/filesystem-6521-Directory-listing-using-C-11-range-based-for-loops-td4570988.html
/**@b Class to allow to iterator over a directory's content with c11 for loop semantics
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

/**@b List the files in directory d
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

/**@b Helper function for listing files recursively
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
	  		files[current] = true;
	  		if(recursive && currentLevel <= levels){
	  			listAllFilesHelper(current, recursive, files,
	  					currentLevel + 1, levels);
	  		}
	  	}else{
	  		files[current] = false;
	  	}
	  }
	}
}

/**@b Function to list all the files of a directory with the option to search recursively and name filtering
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

/**@b List files in a directory with optional recursive search and checking for regex patterns
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

/**@b List files in a directory with optional recursive search and checking for regex patterns
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


/**@b Open a ofstream with filename and checking for file existence
 *
 * @param file the ofstream object to open
 * @param filename The name of the file to open
 * @param overWrite Whether the file should be overwritten if it already exists
 * @param append Whether the file should be appended if it already exists
 * @param exitOnFailure whether program should exit on failure to open the file
 * @todo probably should just remove exitOnFailure and throw an exception instead
 */
inline void openTextFile(std::ofstream& file, std::string filename,
                         bool overWrite, bool append, bool exitOnFailure) {

  if (bfs::exists(filename) && !overWrite) {
    if (append) {
      file.open(filename.data(), std::ios::app);
    } else {
      std::cout << filename << " already exists" << std::endl;
      if (exitOnFailure) {
        exit(1);
      }
    }
  } else {
    file.open(filename.data());
    if (!file) {
      std::cout << "Error in opening " << filename << std::endl;
      if (exitOnFailure) {
        exit(1);
      }
    } else {
      chmod(filename.c_str(),
            S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
  }
}

/**@b convience function to just get the string of the current path from a bfs path object of the current path
 *
 * @return A string of the current path
 */
inline std::string get_cwd() {
	return bfs::current_path().string();
}


/**@b Get home directory, only works if $HOME set in environment
 *
 * @return The home directory if $HOME exists, blank otherwise
 */
inline std::string getHomeStr(){
	char * homeTest1 = getenv("HOME");
	if(homeTest1){
		return homeTest1;
	}else{
		return "";
	}
}


/**@b Get a string of a filename with the extension removed, will preserve path name
 *
 * @param filename The filename to remove the extension
 * @return The filename with the extension removed
 */
inline std::string removeExtension(const std::string& filename) {
	return bfs::path(filename).replace_extension("").string();
}

/**@b Wrapper boost filesystem to get the a string of the extension for a given filename
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

/**@b Wrapper boost filesystem to get the a string of the filename without extension for a given filename
 *
 * @param filename The filename to get the filename for
 * @return The filename (without extention)
 */
inline std::string getFileName(const std::string& filename) {
	return bfs::basename(filename);
}

/**@b Wrapper boost filesystem to get the a string of the parent path for a given filename
 *
 * @param filename A the filename to look for a parent path
 * @return The parent path
 */
inline std::string getPathName(const std::string& filename) {
	return bfs::path(filename).parent_path().string();
}

/**@b Find a filename that doesn't exist by appending _NUM if outFilename exists until outFilename_NUM doesn't exist
 *
 * @param outFilename Filename to check for existance
 * @return A filename for a file that doesn't exist
 */
inline std::string findNonexitantFile(const std::string & outFilename){
	if(bfs::exists(outFilename)){
		std::string fileStub = removeExtension(outFilename);
		std::string extention = getExtension(outFilename);
		uint32_t fileCount = 1;
		std::string newOutFilename = fileStub + "_" + estd::to_string(fileCount) + extention;
		while (bfs::exists(newOutFilename)){
			++fileCount;
			newOutFilename = fileStub + "_" + estd::to_string(fileCount) + extention;
		}
		return newOutFilename;
	}else{
		return outFilename;
	}
}

/**@b make directory with given permissions and check for existence
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
/**@b Make a directory in the given parent directory with the given permissions
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
    std::cout << "Error in making directory " << newDirectoryName << std::endl;
    exit(1);
  }
  return newDirectoryName;
}

/**@b Make several directories in the given parent directory with the given permissions
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

/**@b Copy the contents of a file into a string
 *
 * @param fnp The name of the file
 * @param verbose Whether to print a statement about reading the file
 * @return A string containing the contents of the file
 */
inline static std::string get_file_contents(const bfs::path& fnp, bool verbose) {
	// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
	std::ifstream f(fnp.string(), std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		throw err::str(err::F() << "could not open file" << fnp);
	}
	if (verbose) {
		std::cout << "reading file " << fnp << std::endl;
	}
	std::string ret;
	f.seekg(0, std::ios::end);
	ret.resize(f.tellg());
	f.seekg(0, std::ios::beg);
	f.read(&ret[0], ret.size());
	f.close();
	return ret;
}
/**@b Wraps boost's last_write_time to get the time since last write in std::chrono::time_point object instead of simply time_t
 *
 * @param fnp The filename's path
 * @return The last write time of a file stored in a std::chrono::time_point object
 */
inline sch::time_point<sch::system_clock> last_write_time(const bfs::path & fnp){
	return sch::system_clock::from_time_t(bfs::last_write_time(fnp));
}

/**@b Remove duplicate lines from file, does not preserve order, not optimized for speed
 *
 * @param inputFile The name of the input file
 * @param outputFile The anme of an output file
 * @return The error state of the both the operation of opening the file and closing the file
 */
inline int removeDuplicateLines(const std::string & inputFile,
		const std::string & outputFile){

	/*
	 * adpated from "The C++ programming language", fourth edition
	 * 					 by Bjarne Stroustrup
	 */
	std::ifstream is {inputFile};
	std::set<std::string> content{std::istream_iterator<std::string>{is},
	std::istream_iterator<std::string>{}}; //read in file,
																				//while set ignores duplicates
	std::ofstream out {outputFile};
	std::copy(content.begin(), content.end(),
			std::ostream_iterator<std::string>{out, "\n"}); //output
	return !is || !out; // return the error state
}


} // namespace files
} // namespace bib
