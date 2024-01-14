#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include <string>
#include "nlohmann/json.hpp"


class ConverterJSON
{
public:
    ConverterJSON(
        const std::string config_directory = "config.json",
        const std::string requests_directory = "requests.json",
        const std::string answers_directory = "answers.json");

    nlohmann::json reading_json(const std::string &directory_file,const size_t max_file_size = 105000000)const ; // примрено 100 MB
    nlohmann::json get_config_files_list();
    bool reading_config();
    bool set_numReadThreads(int i);

    std::string config_directory;
    std::string requests_directory;
    std::string answers_directory;


    static const std::string main_project_version;
    static const std::string project_name;

private:
    nlohmann::json config_files_list;
    static bool control_config(const nlohmann::json &json_data,const std::string &version, const std::string &project_name);
    static void set_buffer(std::vector<std::string> &buffer,const size_t &size_file, const int numReadThreads);
    static void reading(const std::string directory_file ,char *buffer,int start,int stop);
    nlohmann::json parse_buffer(const std::vector<std::string> &buffer) const;
    int numReadThreads{1};
    static const unsigned int num_threads;


};




#endif // CONVERTERJSON_H
