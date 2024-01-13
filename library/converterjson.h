#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include <string>
#include "nlohmann/json.hpp"

class ConverterJSON
{
public:
    ConverterJSON(
        const std::string config_directory = "config.json",
        const std::string requests_directory ="requests.json",
        const std::string answers_directory = "answers.json");

    static nlohmann::json reading_json(const std::string &directory_file);
    bool reading_config();

    static bool control_config(const nlohmann::json &json_data,const std::string &version, const std::string &project_name);

    std::vector<std::string> files;
    std::string config_directory;
    std::string requests_directory;
    std::string answers_directory;

};

#endif // CONVERTERJSON_H
