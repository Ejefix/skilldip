#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <array>
#include <thread>
#include <chrono>
#include "version_main.h"
#include <filesystem>
#include <mutex>
#include <cstdlib>
#include "nlohmann/json.hpp"

std::mutex mutex_cerr;

// количество ядер
const unsigned int Skeleton::num_threads = std::thread::hardware_concurrency();
const std::string Skeleton::main_project_version {PROJECT_VERSION};
const std::string Skeleton::project_name{PROJECT_NAME};
int Skeleton::maxThreads;



Skeleton::Skeleton(const int maxThreads):
    config_files_list{std::make_shared<nlohmann::json>()},
    requests_list{std::make_shared<nlohmann::json>()}
{
    Skeleton::maxThreads = maxThreads;
    max_sizeMB = 300;
    if (maxThreads < 1 || maxThreads > Skeleton::num_threads)
        Skeleton::maxThreads = Skeleton::num_threads;
}

Skeleton::~Skeleton()
{

}

void Skeleton::set_max_size_PerThread(float max_sizeMB)
{
    if (max_sizeMB < 0)
        this->max_sizeMB = 1;
    this->max_sizeMB = max_sizeMB;
}

std::shared_ptr<std::vector<std::string>> Skeleton::readFile(const std::string &directory_file, const size_t size_file)const
{
    std::shared_ptr<std::vector<std::string>> buffer = std::make_shared<std::vector<std::string>>();
    set_buffer(*buffer, size_file);

    std::vector<std::thread> threads;

    bool errorOccurred = false;

    for (int i{};i < buffer->size();++i)
    {
        if (i == buffer->size()-1)
        {   try {
                readFileToBuffer(directory_file, &((*buffer)[i][0]), buffer->at(0).size() * i, buffer->at(i).size());
            }
            catch (const std::exception& e) {
                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::lock_guard<std::mutex> lock(mutex_cerr);
                    std::cerr << "error: " << e.what() << '\n';
                }
                buffer->at(i).clear();
            }
            catch (...) {

                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::lock_guard<std::mutex> lock(mutex_cerr);
                    std::cerr << "error "  << '\n';
                }
                buffer->at(i).clear();
            }
        }
        else{
            threads.emplace_back(
                [this,i,&directory_file,&buffer,&errorOccurred](){
                    try {
                        readFileToBuffer(directory_file, &((*buffer)[i][0]), buffer->at(0).size() * i, buffer->at(i).size());
                    }
                    catch (const std::exception& e) {

                        if (!errorOccurred)
                        {
                            errorOccurred = true;
                            std::lock_guard<std::mutex> lock(mutex_cerr);
                            std::cerr << "error: " << e.what() << '\n';
                        }
                        buffer->at(i).clear();
                    }
                    catch (...) {

                        if (!errorOccurred)
                        {
                            errorOccurred = true;
                            std::lock_guard<std::mutex> lock(mutex_cerr);
                            std::cerr << "error "  << '\n';
                        }
                        buffer->at(i).clear();
                    }
                });}
    }
    for (auto& thread : threads) {
        thread.join();
    }
    return buffer;
}






ConverterJSON::ConverterJSON(const int maxThreads)
    :Skeleton{maxThreads}
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
            std::cout << "list files config.json is empty";
            return false;
        }
        if(json_data["config"]["max_responses"].is_number())
        {
            max_responses = json_data["config"]["max_responses"].get<int>();
        }
        else
        {
            max_responses = 0;
        }
        if (max_responses < 1)
            throw std::runtime_error("max_responses no correct");

        time_reading_config = get_data_file_sec(config_directory);
        *config_files_list = json_data["files"];
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
        std::cerr << "error: " << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "error: \n";
    }
    return false;

}


void Skeleton::set_buffer(std::vector<std::string > &buffer,const size_t size_file)const
{

    // тут нету смысла использовать все потоки, мы не сможем преодолеть ограничеия диска
    // по сути вообще на малые файлы нету смысла делать потоки
    // если счёт пойдет на Gb тогда можно открыть еще потоки
    int numReadThreads = 1;

    if (size_file / 300*1024*1024 > 1) // if > 300 Mb
    {
        numReadThreads = size_file / max_sizeMB*1024*1024;

        if (numReadThreads > maxThreads )
            numReadThreads =  maxThreads;
    }

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
    const size_t size_file = std::filesystem::file_size(directory_file);

    if (max_file_size < size_file )
    {
        throw std::runtime_error("File size > " + std::to_string(size_file / 1024 / 1024) + " MB in " + directory_file);
    }
    std::shared_ptr<std::vector<std::string>> buffer = std::make_shared<std::vector<std::string>>();
    buffer = readFile(directory_file,size_file);
    return parse_buffer(*buffer);
}

std::shared_ptr<nlohmann::json> ConverterJSON::get_list_files_config(const int str_size,const bool filter)
{
    try {

        if (time_reading_config < get_data_file_sec(config_directory) && reading_config())
        {
            if(filter)
                filter_files(config_files_list,str_size);
            return config_files_list;
        }
        else{
            if(filter)
                filter_files(config_files_list,str_size);
            return config_files_list;
        }
    }
    catch (const std::exception& e) {

        std::cerr << "error: " << e.what() << '\n';
        return std::make_shared<nlohmann::json>();
    }
    catch (...) {
        return std::make_shared<nlohmann::json>();
    }
}

std::shared_ptr<nlohmann::json> ConverterJSON::get_list_files_requests(const int str_size,const bool filter)
{
    try {
        if (time_reading_requests < get_data_file_sec(requests_directory) )
        {
            nlohmann::json list = reading_json(requests_directory);
            *requests_list = list["requests"];
            if(filter)
                filter_files(requests_list,str_size);
            return requests_list;
        }
        else{
            if(filter)
                filter_files(requests_list,str_size);
            return requests_list;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return std::make_shared<nlohmann::json>();
    }
    catch (...) {
        return std::make_shared<nlohmann::json>();
    }
}

std::shared_ptr<nlohmann::json> ConverterJSON::get_list_files_config(const bool filter)
{
    //вроде в windows сейчас максимум 32000 символов
    return get_list_files_config(40000, filter);

}

std::shared_ptr<nlohmann::json> ConverterJSON::get_list_files_requests(const bool filter)
{
                        // 1 млн
    return get_list_files_requests(1000000,filter);
}

size_t ConverterJSON::get_data_file_sec(const std::string &directory_file)
{
    if (!std::filesystem::exists(directory_file)) {
        throw std::runtime_error("File is missing: " + directory_file);
    }
    // Получаем время последнего изменения файла
    auto ftime = std::filesystem::last_write_time(directory_file);

    // Преобразуем время в тип duration, представляющий количество секунд
    auto duration = ftime.time_since_epoch();

    // Преобразуем duration в секунды
    size_t last_modified_time = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return last_modified_time;
}


void Skeleton::readFileToBuffer(const std::string &directory_file, char *buffer, int start, int stop)const
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

void ConverterJSON::filter_files(std::shared_ptr<nlohmann::json> filter_list, const int str_size)
{
    for (auto it = filter_list->begin(); it != filter_list->end(); )
    {
        if (!it->is_string() || it->get<std::string>().size() > str_size )
        {
           // std::cout << it->get<std::string>();
            it = filter_list->erase(it);
        }
        else
        {
            ++it;
        }
    }
}

nlohmann::json ConverterJSON::parse_buffer(std::vector<std::string> &buffer)const
{
    try {

        nlohmann::json obj_array;
        std::string json_string;
        size_t res{};
        for (const std::string& str : buffer)
            res += str.size();
        json_string.reserve(res);
        int size = buffer.size();
        for (int i{}; i <size;++i ) {

            json_string += buffer[i];
            buffer[i].clear();
           // std::cout << "buffer[i].clear(); " << buffer[i].size();
        }
        obj_array = nlohmann::json::parse(json_string);
        return obj_array;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: \n" << e.what() << '\n';
    }
    return nlohmann::json{};
}

