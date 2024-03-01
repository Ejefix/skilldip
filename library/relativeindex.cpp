#include "relativeindex.h"
#include <iostream>
size_t RelativeIndex::max_relative = 0;

RelativeIndex::RelativeIndex(const std::string &directory_file,
                             const std::vector<std::pair< std::string,  size_t> > &result):
    directory_file{directory_file},result{result}
{
    for (auto &j : result)
    {
        relative += j.second;
    }
    if(relative >  RelativeIndex::max_relative)
    {
        RelativeIndex::max_relative = relative;
    }

}

double RelativeIndex::get_relative()const
{
    return relative;
}

size_t RelativeIndex::get_max_relative()
{
    return RelativeIndex::max_relative;
}

double RelativeIndex::get_Relative_Relevancy()const
{
    return relative/max_relative;
}

std::string RelativeIndex::get_directory_file()const
{
    return directory_file;
}

std::vector<std::pair<std::string, size_t> > RelativeIndex::get_result()const
{
    return result;
}
