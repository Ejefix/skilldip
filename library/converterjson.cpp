#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <array>
#include "version_main.h"
#include <filesystem>
#include <cstdlib>
#include <mutex>

const std::string main_project_version{PROJECT_VERSION};
const std::string project_name{PROJECT_NAME};


size_t Info_file::data_sec(const std::string &directory_file)
{
    if (!std::filesystem::exists(directory_file)) {
        throw std::runtime_error("File is missing: " + directory_file);
    }
    auto ftime = std::filesystem::last_write_time(directory_file); 
    auto duration = ftime.time_since_epoch(); 
    size_t last_modified_time = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return last_modified_time;
}

size_t Info_file::size(const std::string &directory_file)
{
    if (!std::filesystem::exists(directory_file))
    {
        throw std::runtime_error("File is missing: " + directory_file);

    }
    return std::filesystem::file_size(directory_file);

}

//===================================================================================================================

std::shared_ptr<nlohmann::json> ConverterJSON::get_list()
{

    parsing_list();
    return list;
}

void ConfigJSON::filter_str(std::shared_ptr<nlohmann::json> filter_list,int str_size)
{
    for (auto it = filter_list->begin(); it != filter_list->end(); )
    {
        if (!it->is_string() || it->get<std::string>().size() > str_size )
        {
            it = filter_list->erase(it);
        }
        else
        {
            ++it;
        }
    }
}


void ConverterJSON::set_directory(std::string directory)
{
    this->directory = directory;
    time_reading = 1;
}

bool ConverterJSON::update_list()
{
    try {
        if (time_reading < Info_file::data_sec(directory) )
        {
            time_reading = Info_file::data_sec(directory);
            list = std::make_shared<nlohmann::json>(reading_json(directory));
            return true;
        }
        else{
            return false;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        list->clear();
        return false;
    }
    catch (...) {
        std::cerr << "error\n";
        list->clear();
        throw;
    }
}

nlohmann::json ConverterJSON::reading_json(const std::string &directory_file )
{
    try {
        if (std::filesystem::exists(directory_file))
        {          
            nlohmann::json json;
            std::ifstream file(directory_file);
            if (file.is_open()) {
                file >> json;
            }
            return json;
        }
        return nlohmann::json{};
    }
    catch (const std::runtime_error& e) {

        std::cerr << e.what() << '\n';
        return nlohmann::json{};
    }
    catch (...) {
        std::cerr << "error: ConverterJSON::reading_json\n";
        throw;
    }
}

//===================================================================================================================


int ConfigJSON::get_max_responses()
{
    return max_responses;
}

bool ConfigJSON::parsing_config()
{
    try
    {

        if (!control_config(main_project_version , project_name))
        {
            return false;
        }
        if (!list->contains("files")  )
        {
            std::cout << "list files config.json is empty";
            return false;
        }
        if ((*list)["config"]["max_responses"].is_number())
        {
            max_responses = (*list)["config"]["max_responses"].get<int>();
        }
        else
        {
            max_responses = 0;
        }
        if (max_responses < 1)
            throw std::runtime_error("max_responses no correct");
        *list = (*list)["files"];
        filter_str(list,300);
        return true;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        return false;
    }
    catch (...) {
        std::cerr << "error: reading_config()";
        throw;
    }
    return false;
}

ConfigJSON::ConfigJSON():ConverterJSON()
{

    directory = "./config.json";
}

bool ConfigJSON::control_config( const std::string &version, const std::string &project_name)
{
    try {
        if (!list->contains("config")) {
            throw std::runtime_error("Config section is missing in the file");
        }
        if ((*list)["config"]["name"].get<std::string>() !=  project_name)
        {
            throw std::runtime_error("The project name specified in the file is incorrect.");
        }
        if ((*list)["config"]["version"].get<std::string>() != version)
        {
            throw std::runtime_error("version is incorrect -> " + (*list)["config"]["version"].get<std::string>());
        }
        return true;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "error: control_config\n";
        throw;
    }
    return false;

}

void ConfigJSON::parsing_list()
{
    try {
        if(update_list()){
            if(!control_config(main_project_version , project_name))
            {
                list->clear();
                return ;
            }
            parsing_config();
        }
    }
    catch (const std::runtime_error& e) {

        std::cerr << "error: " << e.what() << '\n';
         list->clear();

    }
    catch (...) {
          list->clear();
          throw;
    }
}

//===================================================================================================================

void RequestsJSON::parsing_list()
{
    try {
          if(update_list())
          {

            *list = (*list)["requests"];

          }

    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        list->clear();
    }
    catch (...) {

        std::cerr << "error RequestsJSON::parsing_list\n";
         list->clear();
         throw;
        }
}

RequestsJSON::RequestsJSON():ConverterJSON()
{
    directory = "./requests.json";
}



