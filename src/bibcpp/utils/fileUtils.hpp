#pragma once                                    \

#include <vector>
#include <boost/filesystem.hpp>
namespace bib{
namespace files {

namespace bfs = boost::filesystem;

// from http://boost.2283326.n4.nabble.com/filesystem-6521-Directory-listing-using-C-11-range-based-for-loops-td4570988.html
class dir{
    bfs::path p_;

public:
    inline dir(bfs::path p)
        : p_(p){}

    bfs::directory_iterator begin(){
        return bfs::directory_iterator(p_);
    }

    bfs::directory_iterator end(){
        return bfs::directory_iterator();
    }
};

inline std::vector<bfs::path> filesInFolder(bfs::path d){
    std::vector<bfs::path> ret;

    if(bfs::is_directory(d)){
        for(const auto& e : dir(d)){
            ret.push_back(e);
        }
    }

    return ret;
}

} // namespace files
} // namespace bib
