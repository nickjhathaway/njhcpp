#pragma once
/*
 * FileCache.hpp
 *
 *  Created on: Sep 10, 2016
 *      Author: nick
 */



#include <fstream>
#include <string>
#include <cerrno>
#include <mutex>
#include <boost/filesystem.hpp>
#include "bibcpp/utils/stringUtils.hpp"
#include "bibcpp/files/fileUtilities.hpp" //files::last_write_time
#include "bibcpp/files/fileStreamUtils.hpp" //files::get_file_contents

namespace bib {
namespace files {

/**@brief A file object that holds the contents of a file and updates the contents if the file changed since the last time it was read
 *
 */
class FileCache {
private:
	const bfs::path fnp_; /**< the file path */
	std::string content_; /**< the file content */
	sch::time_point<sch::system_clock> time_; /**< time last read */

	std::mutex mut_; /**< mutex to make updating thread safe*/

	/**@brief Load the content of the file and log the time the file was last edited
	 *
	 */
	void load() {
		content_ = files::get_file_contents(fnp_, false);
		time_ = files::last_write_time(fnp_);
	}

	/**@brief Check to see if the file has changed since
	 *
	 * @return whether an update of the contents is needed
	 */
	bool needsUpdate() const {
		return time_ != files::last_write_time(fnp_);
	}

	/**@brief Update file and return whether the file had to be
	 *
	 * @return Whether the file needed to be updated
	 */
	bool update() {
		std::lock_guard<std::mutex> lock(mut_);
		if (needsUpdate()) {
			load();
			return true;
		}
		return false;
	}

public:

	/**@brief constructor with the content of the file
	 *
	 * @param fnp The file path of the file to be stored
	 */
	FileCache(const bfs::path& fnp) :
			fnp_(fnp) {
		load();
	}

	/**@brief Copy constructor
	 *
	 * @param other FileCache
	 */
	FileCache(const FileCache& other) :
			fnp_(other.fnp_), content_(other.content_), time_(other.time_) {
		update();
	}

	/**@brief Move constructor
	 *
	 * @param other FileCache
	 */
	FileCache(const FileCache&& other) :
			fnp_(std::move(other.fnp_)), content_(std::move(other.content_)), time_(
					std::move(other.time_)) {
		update();
	}

	/**@brief Get the content of the file and update as needed
	 *
	 * @return the current content of the file
	 */
	const std::string& get() {
		update();
		return content_;
	}

	friend class FilesCache;

};



} // namespace files
} // namespace bib



