#include "relativeindex.h"
#include <iostream>
size_t RelativeIndex::max_relative{};

RelativeIndex::RelativeIndex(const std::string &directory_file,
                             const std::vector<std::pair< std::string,  size_t> > &result):
    directory_file{directory_file},result{result}
{
    for (auto &j : result)
    {
        relative += j.second;
    }
    if(relative > max_relative)
        max_relative = relative;
}

double RelativeIndex::get_Relative_Relevancy()
{
    return relative/max_relative;
}
