#ifndef RELATIVEINDEX_H
#define RELATIVEINDEX_H

#include <string>
#include <vector>



class RelativeIndex
{
public:
    RelativeIndex(const std::string &directory_file,
                  const std::vector<std::pair<std::string, size_t>> &result);

    double get_relative();
    static size_t get_max_relative();
    double get_Relative_Relevancy();
    std::string get_directory_file();
    std::vector<std::pair<std::string, size_t>> get_result();
    bool operator >(RelativeIndex &in);
    bool operator <(RelativeIndex &in);
    bool operator == (RelativeIndex &in);
    bool operator >= (RelativeIndex &in);
    bool operator <= (RelativeIndex &in);
    bool operator != (RelativeIndex &in);

private:
    std::string directory_file;
    std::vector<std::pair<std::string, size_t>> result;
    double relative{};
    static size_t max_relative;
};

#endif // RELATIVEINDEX_H