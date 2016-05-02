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
#include "bibcpp/files/filePathUtils.hpp" //join()
#include "bibcpp/files/fileUtilities.hpp"

namespace bib {
namespace files {


// from http://boost.2283326.n4.nabble.com/filesystem-6521-Directory-listing-using-C-11-range-based-for-loops-td4570988.html
/**@brief Class to allow to iterator over a directory's content with c11 for loop semantics
 *
 */
class dir {
	bfs::path p_; /**< directory to iterator over*/

public:
	/**@brief Construct with directory to iterate over
	 *
	 * @param p Path to a directory
	 */
	inline dir(bfs::path p) :
			p_(p) {
	}

	/**@brief Beginning of directory iteration
	 *
	 * @return a boost::filesystem::directory_iterator
	 */
	bfs::directory_iterator begin() {
		return bfs::directory_iterator(p_);
	}

	/**@brief End of directory iteration
	 *
	 * @return a boost::filesystem::directory_iterator, that's just empty
	 */
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
		std::map<bfs::path, bool> & files, uint32_t currentLevel, uint32_t levels) {

	bfs::directory_iterator end_iter;
	if (bfs::exists(dirName) && bfs::is_directory(dirName)) {
		for (const auto & dir_iter : dir(dirName)) {
			bfs::path current = dir_iter.path();
			if (bfs::is_directory(dir_iter.path())) {
				files[bfs::canonical(current)] = true;
				if (recursive && currentLevel <= levels) {
					listAllFilesHelper(current, recursive, files, currentLevel + 1,
							levels);
				}
			} else {
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
		bool recursive, const std::vector<std::string>& contains, uint32_t levels =
				std::numeric_limits<uint32_t>::max()) {
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if (!contains.empty()) {
		std::map<bfs::path, bool> specificFiles;
		for (const auto & f : files) {
			if (checkForSubStrs(f.first.filename().string(), contains)) {
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
		bool recursive, const std::vector<std::regex>& contains, uint32_t levels =
				std::numeric_limits<uint32_t>::max()) {
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if (!contains.empty()) {
		std::map<bfs::path, bool> specificFiles;
		for (const auto & f : files) {
			if (checkForPats(f.first.filename().string(), contains)) {
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
		const std::vector<std::regex>& excludes, uint32_t levels =
				std::numeric_limits<uint32_t>::max()) {
	std::map<bfs::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if (!contains.empty() || !excludes.empty()) {
		std::map<bfs::path, bool> specificFiles;
		for (const auto & f : files) {
			if (checkForPats(f.first.filename().string(), contains)
					&& checkForPatsExclude(f.first.filename().string(), excludes)) {
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
				throw std::runtime_error { ss.str() };
			} else {
				std::cout << ss.str() << std::endl;
			}
		}
	} else {
		file.open(filename.data());
		if (!file) {
			std::stringstream ss;
			ss << "Error in opening " << filename;
			if (exitOnFailure) {
				throw std::runtime_error { ss.str() };
			} else {
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
inline void openTextFile(std::ofstream& file, std::string filename,
		const std::string & extention, bool overWrite, bool append,
		bool exitOnFailure) {
	appendAsNeeded(filename, extention);
	openTextFile(file, filename, overWrite, append, exitOnFailure);
}

/**@brief remove a non empty directory forcibly
 *
 * @param dirName name of directory to remove
 * @return whether or not the removal of the directry was successful
 */
inline bool rmDirForce(const std::string & dirName) {
	if (bfs::is_directory(dirName)) {
		auto files = filesInFolder(dirName);
		for (const auto & f : files) {
			rmDirForce(f.string());
		}
	}
	return bfs::remove(dirName);
}


/**@brief Class to hold parameters to make directory functions
 *
 */
class MkdirPar {
public:
	/**@brief Directory to make, overwrite false by default
	 *
	 * @param dirName The directory to construct
	 */
	explicit MkdirPar(const std::string & dirName) :
			dirName_(dirName) {
	}
	/**@brief Constructor with directory to make and whether to overwrite it if it exists
	 *
	 * @param dirName The directory to construct
	 * @param overWriteDir To overwrite the directory
	 */
	explicit MkdirPar(const std::string & dirName, bool overWriteDir) :
			dirName_(dirName), overWriteDir_(overWriteDir) {
	}

	/**@brief Constructor with directory to make, whether to overwrite it if it exists, and what permissions to give it
	 *
	 * @param dirName The directory to construct
	 * @param overWriteDir To overwrite the directory
	 * @param perms The permissions to give to the directory
	 */
	explicit MkdirPar(const std::string & dirName, bool overWriteDir, mode_t perms) :
			dirName_(dirName), overWriteDir_(overWriteDir), perms_(perms) {
	}

	std::string dirName_; /**< the directory to make*/
	bool overWriteDir_ = false; /**< whether or not to overwrite directory */
	mode_t perms_ = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH; /**< the permissions to set on directory*/
};


/**@brief Make directory with using bib::files::MkdirPar
 *
 * @param pars a bib::files::MkdirPar that will have directory name, whether to overwrite, and permissions
 * @return the status of making the directory
 */
inline int32_t makeDir(const MkdirPar & pars) {
	if (bfs::exists(pars.dirName_)) {
		if (pars.overWriteDir_) {
			bool removalStatus = rmDirForce(pars.dirName_);
			if (!removalStatus) {
				std::stringstream ss;
				ss << "Error in: " << __PRETTY_FUNCTION__
						<< ", when removing directory " << pars.dirName_ << std::endl;
				throw std::runtime_error { ss.str() };
			}
		} else {
			std::stringstream ss;
			ss << "Error in: " << __PRETTY_FUNCTION__ << ", directory " << pars.dirName_
					<< " already exists, use overWrite = true to overwrite contents "
					<< std::endl;
			throw std::runtime_error { ss.str() };
		}
	}
	int32_t directoryStatus = mkdir(pars.dirName_.c_str(), pars.perms_);
	if (directoryStatus != 0) {
		std::stringstream ss;
		ss << "Error in: " << __PRETTY_FUNCTION__ << ", in making directory "
				<< pars.dirName_ << ", mkdir return stats: " << directoryStatus << std::endl;
		throw std::runtime_error { ss.str() };
	}
	return directoryStatus;
}

/**@brief Make a directory in the given parent directory with the given permissions
 *
 * @param parentDirectory The directory wihin which to create the new directory
 * @param newDirectory The name of the new directory to create (can contain the key work TODAY which will be replaced with current date YYYY-MM-DD)
 * @param overWrite Whether to overwrite the directory if it already exists
 * @return The name of the created directory
 */
inline std::string makeDir(const std::string & parentDirectory,
		const MkdirPar & newDirectory) {
	std::string ret = bib::files::join(parentDirectory,
			replaceString(newDirectory.dirName_, "TODAY", getCurrentDate()) + "/");
	MkdirPar params = newDirectory;
	params.dirName_ = ret;
	makeDir(params);
	return ret;
}

/**@brief Make several directories in the given parent directory with the given permissions
 *
 * @param parentDirectory The directory wihin which to create the new directory
 * @param newDirectories The names of the new directories to create
 * @param overWrite Whether to overwrite the directory if it already exists
 * @return The names of the created directories
 */

inline std::vector<std::string> makeDir(std::string parentDirectory,
		std::vector<MkdirPar> newDirectories) {
	std::vector<std::string> ret;
	for (const auto & d : newDirectories) {
		ret.emplace_back(makeDir(parentDirectory, d));
	}
	return ret;
}



/**@brief Make directory with bib::files::MkdirPar if it doesn't exist
 *
 * @param pars a bib::files::MkdirPar that will have directory name, whether to overwrite, and permissions
 */
inline void makeDirP(const MkdirPar & newDirectory) {
	auto toks = tokenizeString(newDirectory.dirName_, "/");
	std::string growingDir = "";
	for (const auto & tok : toks) {
		growingDir += tok + "/";
		if (!bib::files::bfs::exists(growingDir)) {
			auto currentMkPars = MkdirPar(growingDir, false);
			currentMkPars.perms_ = newDirectory.perms_;
			makeDir(currentMkPars);
		}
	}
}

/**@brief Make directory if it doesn't exist
 *
 * @param parentDirectory The directory in which to make the directory
 * @param newDirectory The new directory to make in parentDirectory
 * @return The name of the created directory or the name of the already created directory
 */
inline std::string makeDirP(const std::string &parentDirectory,
		const MkdirPar & newDirectory) {
	std::string ret = bib::files::join(parentDirectory,
			replaceString(newDirectory.dirName_, "TODAY", getCurrentDate()) + "/");
	MkdirPar params = newDirectory;
	params.dirName_ = ret;
	makeDirP(params);
	return ret;
}



}  // namespace files
}  // namespace bib

