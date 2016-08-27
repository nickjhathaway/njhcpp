#pragma once

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

/**@brief A class for holding multiple file caches
 *
 */
class FilesCache {
private:
	std::vector<FileCache> files_; /**< The multiple caches */
	std::string content_; /**< The content */
	std::mutex mut_; /**< mutex to make updating thread safe*/

	/**@brief Load any of the files that need to be reloaded
	 *
	 */
	void load() {
		std::stringstream ss;
		for (auto& f : files_) {
			ss << f.get();
		}
		content_ = ss.str();
	}

	/**@brief Check to see if the file has changed since
	 *
	 * @return whether an update of the contents is needed
	 */
	bool needsUpdate() const {
		bool needsUpdate = false;
		for (auto& f : files_) {
			if (f.needsUpdate()) {
				needsUpdate = true;
			}
		}
		return needsUpdate;
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
	/**@brief Constructor with all the files given
	 *
	 * @param fnps The files to cache
	 */
	FilesCache(std::vector<bfs::path> fnps) {
		for (const auto& p : fnps) {
			files_.emplace_back(p);
		}
		load();
	}

	/**@brief Copy constructor
	 *
	 * @param other FilesCache
	 */
	FilesCache(const FilesCache& other) :
			files_(other.files_), content_(other.content_) {
		update();
	}

	/**@brief Move constructor
	 *
	 * @param other FilesCache
	 */
	FilesCache(const FilesCache&& other) :
			files_(std::move(other.files_)), content_(std::move(other.content_)) {
		update();
	}

	/**@brief Get the cache and check to see if any of the files needed to be updated
	 *
	 * @return The cache of all the files
	 */
	const std::string& get() {
		update();
		return content_;
	}

};
} // namespace files
} // namespace bib
