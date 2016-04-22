#pragma once
/*

 * podVecIO.hpp
 *
 *  Created on: Apr 13, 2016
 *      Author: nick
 */

#include "bibcpp/files/fileUtilities.hpp"
#include "bibcpp/debug/exception.hpp"

namespace bib {
namespace files {


inline void ensureExists(bfs::path fnp){
    if(!bfs::exists(fnp)){
        std::cerr << fnp << ": ERROR: no file found at: "
                  << fnp << "\n";
        std::exit(1);
    }
}

inline void touch(bfs::path fnp){
    if(!bfs::exists(fnp)){
        std::ofstream empty(fnp.string());
        return;
    }
    bfs::last_write_time(fnp, std::time(nullptr));
}

inline void preallocate(bfs::path fnp, const uint64_t numBytes){
    static const uint64_t pageSize = 4096;

    if(bfs::exists(fnp)){
        bfs::remove(fnp);
    }
    touch(fnp);
    bfs::resize_file(fnp, numBytes);

    std::ofstream out(fnp.string(), std::ios::binary|std::ios::out);
    if(!out.is_open()){
        throw bib::err::Exception(bib::err::F() << "could not open file " << fnp);
    }

    // round down to avoid preallocating a file larger than desired
    auto rounded = numBytes - (numBytes % pageSize);

    for(auto i : iter::range(uint64_t(0), rounded, pageSize)){
        out.seekp(i, std::ios_base::beg);
        out.put(0);
    }
    out.close();
}

template <typename T>
void writePODvector(bfs::path fnp, const std::vector<T> d){

    if(bfs::exists(fnp)){
        bfs::remove(fnp);
    }
    touch(fnp);
    uint64_t numBytes = d.size() * sizeof(T);
    bfs::resize_file(fnp, numBytes);

    std::ofstream out(fnp.string(), std::ios::binary|std::ios::out);
    if(!out.is_open()){
        throw bib::err::Exception(bib::err::F() << "could not open file " << fnp);
    }
    auto* cstr = reinterpret_cast<const char*>(d.data());
    out.write(cstr, numBytes);
    out.close();

    //std::cout << "wrote " << fnp << " (" << d.size() << " elements)" << std::endl;
}

template <typename T>
std::vector<T> readPODvector(bfs::path fnp){
    uint64_t numBytes = bfs::file_size(fnp);
    if(numBytes % sizeof(T) != 0){
        throw bib::err::Exception(bib::err::F() << "wrong type for reading file " << fnp);
    }
    uint64_t numElements = numBytes / sizeof(T);

    std::ifstream in(fnp.string(), std::ios::binary|std::ios::in);
    if(!in.is_open()){
        throw bib::err::Exception(bib::err::F() << "could not open file " << fnp);
    }

    std::vector<T> d(numElements);
    in.read(reinterpret_cast<char*>(d.data()), numBytes);
    in.close();

    return d;
}

}  // namespace files


}  // namespace bib
