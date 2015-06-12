#pragma once

#include <fstream>
#include <string>
#include <cerrno>
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/files/fileUtilities.hpp"
#include "bibcpp/debug/exception.hpp"

namespace bib {

namespace bfs = boost::filesystem;
namespace sch = std::chrono;

/**@b A file object that holds the contents of a file and updates the contents if the file changed since the last time it was read
 *
 */
class FileCache {
private:
	const bfs::path fnp_; /**the file path */
	std::string content_; /**the file content */
	sch::time_point<sch::system_clock> time_; /**time last read */
	/**@b Load the content of the file and log the time the file was last edited
	 *
	 */
	void load() {
		content_ = files::get_file_contents(fnp_, false);
		time_ = files::last_write_time(fnp_);
	}

public:
	/**@b empty constructor
	 * @todo should add some safetly to make sure a file path gets set before load is called
	 */
	FileCache() :
			fnp_("") {
		//load();
	}
	/**@b constructor with the content of the file
	 *
	 * @param fnp The file path of the file to be stored
	 */
	FileCache(const bfs::path& fnp) :
			fnp_(fnp) {
		load();
	}
	/**@b Copy constructor
	 *
	 * @param other
	 */
	FileCache(const FileCache& other) :
			fnp_(other.fnp_) {
		load();
	}
	/**@b Get the content of the file and update as needed
	 *
	 * @return the current content of the file
	 */
	const std::string& get() {
		update();
		return content_;
	}
	/**@b Get the content of the file and update as needed and make a replacement of parts of the file
	 *
	 * @param replace The string to replace in the file
	 * @param replacement The replacement for the file
	 * @return The curent content of the file with the replacement done
	 */
	const std::string& get(const std::string & replace,
			const std::string & replacement) {
		update(replace, replacement);
		return content_;
	}
	/**@b Check to see if the file has changed since
	 *
	 * @return
	 */
	bool needsUpdate() const {
		return time_ != files::last_write_time(fnp_);
	}
	/**@b For replacing the parts of the file content with a replacement str
	 *
	 * @param replace The string to replace in the file
	 * @param replacement The replacement for the file
	 */
	void replaceStr(const std::string & replace, const std::string & replacement){
		content_ = replaceString(content_, replace, replacement);
	}
	/**@b Update file and return whether the file had to be
	 *
	 * @return Whether the file needed to be updated
	 */
	bool update(){
		if (needsUpdate()) {
			load();
			return true;
		}
		return false;
	}
	/**@b Update file and return whether the file had to be, also do a string replacement on the file
	 *
	 * @param replace The string to replace in the file
	 * @param replacement The replacement for the file
	 * @return Whether the file needed to be updated
	 */
	bool update(const std::string & replace,
			const std::string & replacement){
		if (needsUpdate()) {
			load();
			replaceStr(replace, replacement);
			return true;
		}
		return false;
	}

};

/**@b A class for holding multiple file caches
 *
 */
class FilesCache {
private:
	std::vector<FileCache> files_; /**The multiple caches */
	std::string content_; /**The content */
	/**@b Load any of the files that need to be reloaded
	 *
	 */
	void load() {
		std::stringstream ss;
		for (auto& f : files_) {
			ss << f.get();
		}
		content_ = ss.str();
	}
public:
	/**@b Constructor with all the files given
	 *
	 * @param fnps The files to cache
	 */
	FilesCache(std::vector<bfs::path> fnps) {
		for (const auto& p : fnps) {
			files_.emplace_back(FileCache(p));
		}
		load();
	}
	/**@b Get the cache and check to see if any of the files needed to be updated
	 *
	 * @return The cache of all the files
	 */
	const std::string& get() {
		bool needsUpdate = false;
		for (auto& f : files_) {
			if (f.update()) {
				needsUpdate = true;
			}
		}
		if(needsUpdate){
			load();
		}
		return content_;
	}
	/**@b Get the cache and check to see if any of the files needed to be updated, doing a replacement on files
	 *
	 * @param replace The string to replace in the file
	 * @param replacement The replacement for the file
	 * @return The cache of all the files with a string replacment done as well
	 */
	const std::string& get(const std::string & replace,
			const std::string & replacement) {
		bool needsUpdate = false;
		for (auto& f : files_) {
			if (f.update(replace, replacement)) {
				needsUpdate = true;
			}
		}
		if(needsUpdate){
			load();
		}
		return content_;
	}
};

} // namespace bib
