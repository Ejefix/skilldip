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


size_t Info::time_file(const std::string &directory_file)
{
    if (!std::filesystem::exists(directory_file)) {
         throw std::runtime_error("File is missing: " + directory_file);
//        std::cerr << "File is missing: " + directory_file << '\n';
//        return size_t{};
    }
    auto ftime = std::filesystem::last_write_time(directory_file); 
    auto duration = ftime.time_since_epoch(); 
    size_t last_modified_time = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return last_modified_time;
}

size_t Info::size_file(const std::string &directory_file)
{
    if (!std::filesystem::exists(directory_file))
    {
        throw std::runtime_error("File is missing: " + directory_file);

    }
    return std::filesystem::file_size(directory_file);

}

//===================================================================================================================

std::shared_ptr<nlohmann::json> ConverterJSON::get_list(bool forcibly)
{

    parsing_list(forcibly);
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


void ConverterJSON::set_directory(const std::string &directory)
{
    time_reading = 1;
    this->directory = directory;

}

std::string ConverterJSON::get_directory()
{
    return directory;
}

bool ConverterJSON::update_list(bool forcibly)
{
    try {

        if(forcibly)
        {
            list = std::make_shared<nlohmann::json>(reading_json(directory));
            return true;
        }
        if (time_reading < Info::time_file(directory) )
        {           
            time_reading = Info::time_file(directory);
            list = std::make_shared<nlohmann::json>(reading_json(directory));
            return true;
        }
        else{
            return false;
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';

        return false;
    }
    catch (...) {
        std::cerr << "error\n";

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

bool ConverterJSON::saveToFile(const std::string &directory_file, const nlohmann::ordered_json &js)
{
    std::ofstream file(directory_file);
    if (!file.is_open()) {
        std::cerr << "File creation error " << directory_file << '\n';
        return false;
    }
    file << js.dump(4);
    file.close();
    return true;
}

std::string Info::get_time()
{
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}


//===================================================================================================================


int ConfigJSON::get_max_responses()
{
    return max_responses;
}

void ConfigJSON::set_directory(const std::string &directory)
{
    if(std::filesystem::exists(info_file)) {
        if(std::filesystem::remove(info_file))
            ConverterJSON::set_directory(directory);
        else {
            std::cerr << "File deletion error: " << info_file <<'\n';
            std::cerr << "Do not save the file directory : " << directory <<'\n';
        }
    }
    else
    {
        ConverterJSON::set_directory(directory);
    }

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

bool ConfigJSON::update_list(bool forcibly)
{
    if(forcibly)
    {
        return ConverterJSON::update_list(forcibly);
    }
    size_t info_file_;
    size_t directory_;

    if(std::filesystem::exists(info_file) && std::filesystem::exists(directory))
    {
        info_file_ = Info::time_file(info_file);
        directory_ = Info::time_file(directory);
        if ( info_file_  >  directory_)
        {
            return false;
        }
    }

    return ConverterJSON::update_list(forcibly);


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

bool ConfigJSON::parsing_list(bool forcibly)
{
    try {
        if(update_list(forcibly)){                      
            if(!control_config(main_project_version , project_name))
            {
                if(list !=nullptr)
                    list->clear();
                return false;
            }
            if(parsing_config())
            {
                if(!std::filesystem::exists("./js"))
                {
                    std::filesystem::create_directories("./js");
                }
                nlohmann::ordered_json js = true;
                return ConverterJSON::saveToFile(info_file, js);
            }
            return false;
        }
        return false;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        return false;
    }
    catch (...) {
        throw;
    }
}

//===================================================================================================================

bool RequestsJSON::parsing_list(bool forcibly)
{
    try {

          if(update_list(forcibly))
          {
            *list = (*list)["requests"];
            if(!std::filesystem::exists("./js"))
            {
                std::filesystem::create_directories("./js");
            };
            nlohmann::ordered_json js = true;
            return ConverterJSON::saveToFile(info_file, js);
          }
          return false;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "error: " << e.what() << '\n';
        return false;
    }
    catch (...) {

        std::cerr << "error RequestsJSON::parsing_list\n";       
        throw;
    }
}

bool RequestsJSON::update_list(bool forcibly)
{
    if(forcibly)
    {
        return ConverterJSON::update_list(forcibly);
    }
    size_t info_file_;
    size_t directory_;

    if(std::filesystem::exists(info_file) && std::filesystem::exists(directory))
    {
        info_file_ = Info::time_file(info_file);
        directory_ = Info::time_file(directory);
        if ( info_file_  >  directory_)
        {
            return false;
        }
    }

    return ConverterJSON::update_list(forcibly);

}

RequestsJSON::RequestsJSON():ConverterJSON()
{

    directory = "./requests.json";
}

void RequestsJSON::set_directory(const std::string &directory)
{
    if(std::filesystem::exists(info_file)) {
         if(std::filesystem::remove(info_file))
            ConverterJSON::set_directory(directory);
         else {
            std::cerr << "File deletion error: " << info_file <<'\n';
            std::cerr << "Do not save the file directory : " << directory <<'\n';
         }
    }
    else
    {
         ConverterJSON::set_directory(directory);
    }
}




