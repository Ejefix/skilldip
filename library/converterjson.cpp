#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include "version_main.h"

// количество ядер
static const unsigned int num_threads = std::thread::hardware_concurrency();
const std::string ConverterJSON::main_project_version {PROJECT_VERSION};
const std::string ConverterJSON::project_name{PROJECT_NAME};


ConverterJSON::ConverterJSON(const std::string config_directory,
                             const std::string requests_directory,
                             const std::string answers_directory)
    :config_directory{config_directory},
    requests_directory{requests_directory},
    answers_directory{answers_directory}
{

}

bool ConverterJSON::reading_config()
{
    try
    {
        nlohmann::json files_array;
        {
            nlohmann::json json_data = reading_json(config_directory);
            if (!control_config(json_data, main_project_version , project_name))
            {
                return false;
            }
            if (!json_data.contains("files")  )
            {
                throw std::runtime_error("files is empty");
            }
            files_array = json_data["files"];
        }
        size_t size = files_array.size();
        std::cout << "size " << size << "\n";
         if(size < 1)
             {return false;}

        files.reserve(size);
        for (auto &file: files_array)
        {
            std::cout << "file " << file << "\n";
            files.push_back(file);
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return false;
    }
    catch (...) {
        return false;
    }

}

bool ConverterJSON::control_config(const nlohmann::json &json_data, const std::string &version, const std::string &project_name)
{

        try {
            if (!json_data.contains("config") && !json_data["config"].contains("name") )
            {
                throw std::runtime_error("config file is empty");
            }
            if (!json_data["config"].contains("name") )
            {
                throw std::runtime_error("name is empty");
            }
            if (json_data["config"]["name"].get<std::string>() !=  project_name)
            {
                throw std::runtime_error("The project name specified in the file is incorrect.");
            }
            if (json_data["config"]["version"].get<std::string>() != version)
            {
                throw std::runtime_error("version is incorrect");
            }
        }
        catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << std::endl;
            return false;
        }
        catch (...) {
            return false;
        }
        return true;

}

nlohmann::json ConverterJSON::reading_json(const std::string &directory_file)
{
    nlohmann::json json_data;
    std::ifstream input_file(directory_file);
    if (!input_file.is_open()) {
        throw std::runtime_error("File is missing: " + directory_file);
    } else {
        input_file >> json_data;
        input_file.close();
    }
    return json_data;
}




