#pragma once

#include <boost/filesystem.hpp>
#include <chrono>

namespace bib {
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

} // namespace files
} // namespace bib


