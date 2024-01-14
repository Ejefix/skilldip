#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include "version_main.h"
#include <filesystem>
#include <mutex>


std::mutex mutex_cerr;

// количество ядер
const unsigned int ConverterJSON::num_threads = std::thread::hardware_concurrency();
const std::string ConverterJSON::main_project_version {PROJECT_VERSION};
const std::string ConverterJSON::project_name{PROJECT_NAME};


ConverterJSON::ConverterJSON(const std::string config_directory,
                             const std::string requests_directory,
                             const std::string answers_directory)
    :config_directory{config_directory},
    requests_directory{requests_directory},
    answers_directory{answers_directory},
    numReadThreads {1}
{

}

bool ConverterJSON::reading_config()
{
    try
    {     

        nlohmann::json json_data = reading_json(config_directory);
        if (!control_config(json_data, main_project_version , project_name))
        {
            return false;
        }
        if (!json_data.contains("files")  )
        {
            throw std::runtime_error("list files is empty");
        }
        config_files_list = json_data["files"];
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';

    }
    catch (...) {
        std::cerr << "error: reading_config()";
    }
    return false;
}

bool ConverterJSON::control_config(const nlohmann::json &json_data, const std::string &version, const std::string &project_name)
{

    try {
        if (!json_data.contains("config")) {
            throw std::runtime_error("Config section is missing in the file");
        }
        if (json_data["config"]["name"].get<std::string>() !=  project_name)
        {
            throw std::runtime_error("The project name specified in the file is incorrect.");
        }
        if (json_data["config"]["version"].get<std::string>() != version)
        {
            throw std::runtime_error("version is incorrect");
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;

    }
    catch (...) {
        std::cerr << "error: ";
    }
    return false;

}

bool ConverterJSON::set_numReadThreads(int i)
{
    if (i > ConverterJSON::num_threads || i < 1)
        return false;

    numReadThreads = i;
    return true;
}

void ConverterJSON::set_buffer(std::vector<std::string > &buffer,const size_t &size_file,const int numReadThreads)
{

    // тут нету смысла использовать все потоки, мы не сможем преодолеть ограничеия диска
    // по сути вообще на малые файлы нету смысла делать потоки
    // если счёт пойдет на Gb тогда можно открыть еще потоки
    std::cout << "потоки для чтения "<< numReadThreads << "\n";
            size_t resize_string = size_file / numReadThreads;
    size_t remainder = size_file % numReadThreads;
    for (int i {}; i < numReadThreads - 1 ;++i)
    {
        std::string j;
        j.resize(resize_string);
        buffer.push_back(j);
    }
    std::string j;
    j.resize(resize_string + remainder);
    buffer.push_back(j);
}

nlohmann::json ConverterJSON::reading_json(const std::string &directory_file,const size_t max_file_size)const
{

    size_t size_file = std::filesystem::file_size(directory_file);

    if (max_file_size < size_file )
    {
        throw std::runtime_error("File size > 100 MB: " + directory_file);
    }
    std::vector<std::string> buffer;
    set_buffer(buffer,size_file, numReadThreads);

    std::vector<std::thread> threads;

    bool errorOccurred = false;

    for (int i{};i < buffer.size();++i)
    {
        if (i == buffer.size()-1)
        {   try {
                reading(directory_file, &buffer[i][0], buffer[0].size()*i, buffer[i].size());
            }
            catch (const std::exception& e) {
                std::lock_guard<std::mutex> lock(mutex_cerr);
                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::cerr << "error: " << e.what() << std::endl;
                }
            }
            catch (...) {
                std::lock_guard<std::mutex> lock(mutex_cerr);
                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::cerr << "error "  << std::endl;
                }
            }
        }
        else{
            threads.emplace_back(
                [i,directory_file,&buffer,&errorOccurred](){
                    try {
                        reading(directory_file, &buffer[i][0], buffer[0].size()*i, buffer[i].size());
                    }
                    catch (const std::exception& e) {
                        std::lock_guard<std::mutex> lock(mutex_cerr);
                        if (!errorOccurred)
                        {
                            errorOccurred = true;
                            std::cerr << "error: " << e.what() << std::endl;
                        }
                    }
                    catch (...) {
                        std::lock_guard<std::mutex> lock(mutex_cerr);
                        if (!errorOccurred)
                        {
                            errorOccurred = true;
                            std::cerr << "error "  << std::endl;
                        }
                    }
                });}
    }
    for (auto& thread : threads) {
        thread.join();
    }

    if (!errorOccurred)
    {
        return parse_buffer(buffer);
    }
    return nlohmann::json{};
}

nlohmann::json ConverterJSON::get_config_files_list()
{
    return config_files_list;
}





void ConverterJSON::reading(const std::string directory_file, char *buffer, int start, int stop)
{
    std::ifstream input_file(directory_file, std::ios::binary);
    if (!input_file.is_open()) {
        throw std::runtime_error("File is missing: " + directory_file);
    } else
    {
        input_file.seekg(start, std::ios::beg); // перемещаем на позицию
        input_file.read(buffer, stop);
    }
}

nlohmann::json ConverterJSON::parse_buffer(const std::vector<std::string> &buffer) const
{
    nlohmann::json obj_array;
    std::string json_string;

    for (const std::string& str : buffer) {

        json_string += str;
    }

    try {
        obj_array = nlohmann::json::parse(json_string);
        return obj_array;
    } catch (const nlohmann::json::parse_error& e) {

        std::cerr << "JSON parsing error: " << e.what() << '\n';

    }
    return nlohmann::json{};
}
