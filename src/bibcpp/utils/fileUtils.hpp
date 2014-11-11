#pragma once

#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp"

namespace bib {
namespace files {

namespace bfs = boost::filesystem;

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
			ret.push_back(e);
		}
	}

	return ret;
}

/**@ Helper function for listing files recursively
 *
 * @param dirName The name of directory to search
 * @param recursive Whether the search should be recursive
 * @param files The map container in which to store results
 * @param currentLevel The current level of the search if recursive
 * @param levels The max level to search to, level 1 be just the current directory, 2 being the contents of directories of the current dir, etc.
 */
inline void listAllFilesHelper(const boost::filesystem::path & dirName, bool recursive,
		std::map<boost::filesystem::path, bool> & files,
		uint32_t currentLevel,
		uint32_t levels){

	boost::filesystem::directory_iterator end_iter;
	if(boost::filesystem::exists(dirName) && boost::filesystem::is_directory(dirName)){
	  for(const auto & dir_iter : dir(dirName)){
	  	boost::filesystem::path current = dir_iter.path();
	  	if(boost::filesystem::is_directory(dir_iter.path())){
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
inline std::map<boost::filesystem::path, bool> listAllFiles(const std::string & dirName,
		bool recursive, const std::vector<std::string>& contains,
		uint32_t levels = std::numeric_limits<uint32_t>::max()){
	std::map<boost::filesystem::path, bool> files;
	listAllFilesHelper(dirName, recursive, files, 1, levels);
	if(!contains.empty()){
		std::map<boost::filesystem::path, bool> specificFiles;
		for(const auto & f : files){
			if(checkForSubStrs(f.first.string(), contains)){
				specificFiles.emplace(f);
			}
		}
		return specificFiles;
	}
	return files;
}

} // namespace files
} // namespace bib
