#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <array>
#include "version_main.h"
#include <filesystem>
#include <cstdlib>


namespace THReads {
std::mutex mutex_cerr;
std::mutex insert_lock;
}


const std::string Skeleton::main_project_version {PROJECT_VERSION};
const std::string Skeleton::project_name{PROJECT_NAME};


Skeleton::Skeleton(int maxThreads)
{
    this->maxThreads = maxThreads;
    max_sizeMB = 300;
    if (maxThreads < 1 || maxThreads > THReads::num_threads)
        this->maxThreads = THReads::num_threads;
}

Skeleton::~Skeleton()
{

}

size_t Skeleton::get_data_file_sec(const std::string &directory_file)
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

size_t Skeleton::get_file_size(const std::string &directory_file)
{
    return std::filesystem::file_size(directory_file);
}

void Skeleton::set_max_size_PerThread(float max_sizeMB)
{
    if (max_sizeMB < 0)
        this->max_sizeMB = 1;
    this->max_sizeMB = max_sizeMB;
}

std::shared_ptr<std::vector<std::string>> Skeleton::readFile(const std::string &directory_file)const
{
    try {
    size_t size_file = get_file_size(directory_file);
    const auto buffer = std::make_shared<std::vector<std::string>>();

    set_buffer(*buffer, size_file);

    std::vector<std::thread> threads;

    size_t size = buffer->size();
    bool errorOccurred = false;

    for (int i{};i < size;++i)
    {
        if (i == size-1)
        {   try {
                //std::cerr << " readFileToBuffer \n";
                readFileToBuffer(directory_file, &((*buffer)[i][0]), buffer->at(0).size() * i, buffer->at(i).size());
            }
            catch (const std::exception& e) {
                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
                    std::cerr << "error: " << e.what() << '\n';
                }
                buffer->at(i).clear();
            }
            catch (...) {

                if (!errorOccurred)
                {
                    errorOccurred = true;
                    std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
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
                            std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
                            std::cerr << "error: " << e.what() << '\n';
                        }
                        buffer->at(i).clear();
                    }
                    catch (...) {

                        if (!errorOccurred)
                        {
                            errorOccurred = true;
                            std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
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
    catch (...) {
    std::cerr << "file " << directory_file << " is missing \n";
    }
    return std::make_shared<std::vector<std::string>>();
}

void Skeleton::set_buffer(std::vector<std::string > &buffer,const size_t size_file)const
{

    // тут нету смысла использовать все потоки, мы не сможем преодолеть ограничеия диска
    // по сути вообще на малые файлы нету смысла делать потоки
    // если счёт пойдет на Gb тогда можно открыть еще потоки
    size_t numReadThreads = 1;

    numReadThreads = static_cast<size_t>(size_file / (max_sizeMB*1024*1024));
   // std::cerr << numReadThreads << " numReadThreads \n";

    if (numReadThreads > maxThreads )
    numReadThreads =  maxThreads;
    if (numReadThreads < 1) {
    numReadThreads = 1;
    }

   // std::cerr << numReadThreads << " set_buffer \n";
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

//===================================================================================================================

ConverterJSON::ConverterJSON(int maxThreads)
    :Skeleton{maxThreads},config_files_list{std::make_shared<nlohmann::json>()},requests_list{std::make_shared<nlohmann::json>()}
{
    update_lists();
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

nlohmann::json ConverterJSON::reading_json(const std::string &directory_file,const size_t max_file_size)const
{
    const size_t size_file = get_file_size(directory_file);
    if (max_file_size < size_file )
    {
        throw std::runtime_error("File size > " + std::to_string(size_file / 1024 / 1024) + " MB in " + directory_file);
    }
    auto buffer = std::make_shared<std::vector<std::string>>();
    buffer = readFile(directory_file);
    return parse_buffer(*buffer);
}

void ConverterJSON::get_list_files_config(int str_size,bool filter)
{
    try {

        if (time_reading_config < get_data_file_sec(config_directory) && reading_config())
        {
            if(filter)
                filter_files(config_files_list,str_size);
        }
        else{
            if(filter)
                filter_files(config_files_list,str_size);                    
        }
    }
    catch (const std::exception& e) {

        std::cerr << "error: " << e.what() << '\n';

    }
    catch (...) {}
}

void ConverterJSON::get_list_files_requests(int str_size,bool filter)
{
    try {
        if (time_reading_requests < get_data_file_sec(requests_directory) )
        {
            nlohmann::json list = reading_json(requests_directory);
            *requests_list = list["requests"];
            if(filter)
                filter_files(requests_list,str_size);          

        }
        else{
            if(filter)
                filter_files(requests_list,str_size);           
        }
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';

    }
    catch (...) {}
}



void ConverterJSON::update_lists()
{

    get_list_files_config(str_size_config,filter_config);
    get_list_files_requests(str_size_requests,filter_requests);
}

void ConverterJSON::set_filter_configJSON(int str_size, bool filter)
{
    if (str_size > 5)
    str_size_config = str_size;
    filter_config = filter;
    settingsChanged = true;
}

void ConverterJSON::set_filter_configJSON(int str_size)
{
    set_filter_configJSON(str_size,filter_config);
}

void ConverterJSON::set_filter_configJSON(bool filter)
{
    set_filter_configJSON(str_size_config,filter);
}

void ConverterJSON::set_filter_requestsJSON(int str_size,bool filter)
{
    if (str_size > 5)
    str_size_requests = str_size;
    filter_requests = filter;
    settingsChanged = true;
}

void ConverterJSON::set_filter_requestsJSON(int str_size)
{
    set_filter_requestsJSON(str_size,filter_requests);
}

void ConverterJSON::set_filter_requestsJSON(bool filter)
{
    set_filter_requestsJSON(str_size_requests,filter);
}

void ConverterJSON::filter_files(std::shared_ptr<nlohmann::json> filter_list,int str_size)
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
    std::string json_string;
    try {

        nlohmann::json obj_array;
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
        buffer.clear();
        return obj_array;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: \n" << e.what() << '\n';
        buffer.clear();
    }
    catch (...) {
        std::cerr << "JSON parsing error: \n" << '\n';
        buffer.clear();
    }
    return nlohmann::json{};
}

