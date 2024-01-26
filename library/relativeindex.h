#ifndef RELATIVEINDEX_H
#define RELATIVEINDEX_H

#include <string>
#include <vector>



class RelativeIndex
{
public:
    RelativeIndex(const std::string &directory_file,
                  const std::vector<std::pair<std::string, size_t>> &result);

    double get_Relative_Relevancy();
    const std::string directory_file;
    const std::vector<std::pair<std::string, size_t>> result;
    double relative{};
    static size_t max_relative;
};

#endif // RELATIVEINDEX_H
