#pragma once
/*
 * md5Utils.hpp
 *
 *  Created on: Sep 23, 2016
 *      Author: nick
 */


#include "njhcpp/md5/md5.hpp"
#include "njhcpp/files.h"

namespace njh {

inline std::ostream& operator<<(std::ostream& out, MD5 md5)
{
  return out << md5.hexdigest();
}




inline std::string md5(const std::string str)
{
    MD5 md5 = MD5(str);

    return md5.hexdigest();
}

inline std::string md5File(const files::bfs::path & fnp)
{
    MD5 md5 = MD5(files::get_file_contents(fnp, false));

    return md5.hexdigest();
}

}  // namespace njh




