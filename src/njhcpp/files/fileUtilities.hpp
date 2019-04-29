#pragma once

#include <boost/filesystem.hpp>
#include <chrono>
#include "njhcpp/debug/exception.hpp"

namespace njh {
/**@brief Namespace to hold operations dealing with files and dealing with the filesystem
 *
 */
namespace files {

namespace bfs = boost::filesystem;
namespace sch = std::chrono;


/**@brief Wraps boost's last_write_time to get the time since last write in std::chrono::time_point object instead of simply time_t
 *
 * @param fnp The filename's path
 * @return The last write time of a file stored in a std::chrono::time_point object
 */
inline sch::time_point<sch::system_clock> last_write_time(
		const bfs::path & fnp) {
	return sch::system_clock::from_time_t(bfs::last_write_time(fnp));
}

/**@brief A test to see which file was modified last, will return true if file2 was modified last
 *
 * @param file1 The first file
 * @param file2 The second file
 * @return true if file2 was modified last
 */
inline bool firstFileIsOlder(
		const bfs::path & file1,
		const bfs::path & file2) {
	return njh::files::last_write_time(file1) < njh::files::last_write_time(file2);
}

/**@brief Checks to see if a list of files are all newer than anther file
 *
 * @param files the files to check
 * @param checkFile the check file to check against
 * @return true if all files are new
 * @todo consider a check to see if files is empty, also this will throw if any of the files don't exist
 */
inline bool allFilesNewerThanCheck(
		const std::vector<bfs::path> & files,
		const bfs::path & checkFile) {
	return std::all_of(files.begin(), files.end(), [&checkFile](const bfs::path & fnp){
		return njh::files::last_write_time(checkFile) < njh::files::last_write_time(fnp);
	});
}

/**@brief Checks to see if a list of files exist are all newer than anther file
 *
 * @param files the files to check
 * @param checkFile the check file to check against
 * @return true if all files are new
 * @todo consider a check to see if files is empty and if check file exists
 */
inline bool allFilesExistAndNewerThanCheck(
		const std::vector<bfs::path> & files,
		const bfs::path & checkFile) {
	return std::all_of(files.begin(), files.end(), [&checkFile](const bfs::path & fnp){
		return bfs::exists(fnp) ? njh::files::last_write_time(checkFile) < njh::files::last_write_time(fnp) : false;
	});
}


/**@brief A check that will throw if a file doesn't exist
 *
 * @param fnp the file to check for
 */
inline void ensureExists(bfs::path fnp) {
	if (!bfs::exists(fnp)) {
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ": ERROR: no file found at: " << fnp << "\n";
		throw std::runtime_error{ss.str()};
	}
}

/**@brief Similar to the unix touch, if file exists change mod time to now, if doesn't exist create it
 *
 * @param fnp The file to touch
 */
inline void touch(bfs::path fnp) {
	if (!bfs::exists(fnp)) {
		std::ofstream empty(fnp.string());
		return;
	}
	bfs::last_write_time(fnp, std::time(nullptr));
}

/**@brief Preallocate an empty file to be a certain size
 *
 * @param fnp The file to preallocate, will overwrite if it exists
 * @param numBytes the number of bytes to size the file to
 */
inline void preallocate(bfs::path fnp, const uint64_t numBytes) {
	static const uint64_t pageSize = 4096;

	if (bfs::exists(fnp)) {
		bfs::remove(fnp);
	}
	touch(fnp);
	bfs::resize_file(fnp, numBytes);

	std::ofstream out(fnp.string(), std::ios::binary | std::ios::out);
	if (!out.is_open()) {
		throw njh::err::Exception(njh::err::F() << "could not open file " << fnp);
	}

	// round down to avoid preallocating a file larger than desired
	auto rounded = numBytes - (numBytes % pageSize);

	for (auto i : iter::range(uint64_t(0), rounded, pageSize)) {
		out.seekp(i, std::ios_base::beg);
		out.put(0);
	}
	out.close();
}

/**@brief is to see if a file is empty, will throw if file is empty
 *
 * @param fnp the file path
 * @return true if file is empty
 */
inline bool isFileEmpty(const bfs::path & fnp){
	if(!bfs::exists(fnp)){
		std::stringstream ss;
		ss << __PRETTY_FUNCTION__ << ", error file: " << fnp << " doesn't exist" << std::endl;
	}
	if (0 == bfs::file_size(fnp)) {
		return true;
	}
	return false;
}


#if defined( __APPLE__ ) || defined( __APPLE_CC__ ) || defined( macintosh ) || defined( __MACH__ )
	static const uint32_t NormalDefaultOpenFileLimit = 254; /**< The maximum number of files that normally allowed to be open by default for current Operating System */
#else
	static const uint32_t NormalDefaultOpenFileLimit = 1024; /**< The maximum number of files that normally allowed to be open by default for current Operating System */
#endif




} // namespace files
} // namespace njh


