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
/**@brief Namespace to hold operations dealing with files and dealing with the filesystem
 *
 */
namespace files {

namespace bfs = boost::filesystem;
namespace sch = sch;

/**@brief Return full path name even if the path doesn't actually exists
 *
 * @param path The path to gt the full path name for
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

/**@brief  append to a str only if need
 *
 * @param str The string to append to
 * @param app What to append to str
 * @return A reference to the str
 */
inline std::string & appendAsNeeded(std::string & str, const std::string & app){
	if(!endsWith(str,app)){
		str.append(app);

	}
	return str;
}

/**@brief Return str appended if required
 *
 * @param str The str to append to
 * @param app What to append to the str
 * @return str appended with app only if it is not already appended
 */
inline std::string appendAsNeededRet(std::string str, const std::string & app){
	if(!endsWith(str,app)){
		str.append(app);
	}
	return str;
}

/**@brief Joiningng parent path with child path with a unix dir separator if needed
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

/**@brief convience function to just get the string of the current path from a bfs path object of the current path
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
	char * homeTest1 = getenv("HOME");
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
		throw err::Exception(err::F() << "could not open file" << fnp);
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
/**@brief Wraps boost's last_write_time to get the time since last write in std::chrono::time_point object instead of simply time_t
 *
 * @param fnp The filename's path
 * @return The last write time of a file stored in a std::chrono::time_point object
 */
inline sch::time_point<sch::system_clock> last_write_time(const bfs::path & fnp){
	return sch::system_clock::from_time_t(bfs::last_write_time(fnp));
}

/**@brief Remove duplicate lines from file, does not preserve order, not optimized for speed
 *
 * @param inputFile The name of the input file
 * @param outputFile The name of an output file
 * @return The error state of the both the operation of opening the file and closing the file
 */
inline int removeDuplicateLines(const std::string & inputFile,
		const std::string & outputFile, bool overWrite){

	/*
	 * adpated from "The C++ programming language", fourth edition
	 * 					 by Bjarne Stroustrup
	 */
	std::ifstream is {inputFile};
	std::set<std::string> content{std::istream_iterator<std::string>{is},
	std::istream_iterator<std::string>{}}; //read in file,
																				//while set ignores duplicates
	std::ofstream out;
	openTextFile(out, outputFile, overWrite, false, true);
	std::copy(content.begin(), content.end(),
			std::ostream_iterator<std::string>{out, "\n"}); //output
	return !is || !out; // return the error state
}

/**@brief Get the first line of a file, throws if there is an error in opening a file
 *
 * @param filename The name of the file
 * @return The first line of the file
 */
inline std::string getFirstLine(const std::string &filename) {
  std::string currentLine;
  std::ifstream textFile(filename.c_str());
  if (!textFile) {
  	throw std::runtime_error{"Error in opening " + filename};
  }
  std::getline(textFile, currentLine);
  return currentLine;
}


/**@brief Get the last line of a file
 *
 * @param filename Name of the file to extract the last line from
 * @return A std::string object containing the last line of the file
 */
inline std::string getLastLine(const std::string & filename) {
	/*
	 Last line from the file, from http://www.programmersbook.com/page/7/Get-last-line-from-a-file-in-C/
	 */
	std::ifstream read(filename, std::ios_base::ate); //open file
	if(!read){
		throw std::runtime_error{bib::bashCT::boldRed("Error in opening " + filename)};
	}
	std::string ret;
	int length = 0;
	char c = '\0';
	if (read) {
		length = read.tellg(); //Get file size
		// loop backward over the file
		for (int i = length - 2; i > 0; i--) {
			read.seekg(i);
			c = read.get();
			if (c == '\r' || c == '\n') //new line?
				break;
		}
		std::getline(read, ret); //read last line
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
		const std::string & outFilename, const std::string outFileExt,
		bool overWrite, bool append, bool exitOnFailure) {
	if (outFilename != "") {
		openTextFile(outFile, outFilename, outFileExt, overWrite,
				append, exitOnFailure);
		return outFile.rdbuf();
	} else {
		return std::cout.rdbuf();
	}
}

} // namespace files
} // namespace bib


