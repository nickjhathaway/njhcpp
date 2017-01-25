#pragma once
/*
 * FilesCache.hpp
 *
 *  Created on: Sep 10, 2016
 *      Author: nick
 */



#include "bibcpp/files/fileObjects/FileCache.hpp"

namespace bib {
namespace files {

/**@brief A class for holding multiple file caches
 *
 */
class FilesCache {
private:
	std::vector<FileCache> files_; /**< The multiple caches */
	std::string content_; /**< The content */
	std::mutex mut_; /**< mutex to make updating thread safe*/
	bool needsUpdate_ = false; /**< indicator for whether the content needs to be update or not*/

	/**@brief Load any of the files that need to be reloaded
	 *
	 */
	void load() {
		std::stringstream ss;
		for (auto& f : files_) {
			ss << f.get();
		}
		content_ = ss.str();
		needsUpdate_ = false;
	}

	/**@brief Check to see if the file has changed since
	 *
	 * @return whether an update of the contents is needed
	 */
	bool needsUpdate() {
		for (auto& f : files_) {
			if (f.needsUpdate()) {
				needsUpdate_ = true;
			}
		}
		return needsUpdate_;
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
			files_(other.files_),
			content_(other.content_),
			needsUpdate_(other.needsUpdate_){
		update();
	}

	/**@brief Move constructor
	 *
	 * @param other FilesCache
	 */
	FilesCache(const FilesCache&& other) :
			files_(std::move(other.files_)),
			content_(std::move(other.content_)),
			needsUpdate_(other.needsUpdate_){
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

	/**@brief Add file to the cache of files
	 *
	 * @param file the path to the file to add
	 */
	void addFile(const bfs::path & file){
		files_.emplace_back(file);
		needsUpdate_ = true;
	}

	/**@brief add files to the cache of files
	 *
	 * @param files a vector of file paths to add to the cache
	 */
	void addFiles(const std::vector<bfs::path> & files){
		for(const auto & file : files){
			addFile(file);
		}
	}

};


} // namespace files
} // namespace bib


