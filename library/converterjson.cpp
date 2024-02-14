#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <array>
#include "version_main.h"
#include <filesystem>
#include <cstdlib>
#include <mutex>


std::mutex mutex_cerr;


const std::string main_project_version {PROJECT_VERSION};
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
    return std::filesystem::file_size(directory_file);
}

//===================================================================================================================


std::shared_ptr<std::vector<std::string>> ReadFile::readFile(const std::string &directory_file,int maxThreads,  size_t max_sizeMB)
{

    try {
        if (maxThreads < 1 || maxThreads > THReads::num_threads)
            maxThreads = THReads::num_threads;
        if (max_sizeMB < 300)
            max_sizeMB = 300;
        size_t size_file = Info_file::size(directory_file);
        const auto buffer = std::make_shared<std::vector<std::string>>();

        set_buffer(*buffer, size_file,maxThreads,max_sizeMB );

        std::vector<std::thread> threads;

        size_t size = buffer->size();
        bool errorOccurred = false;

        for (int i{};i < size;++i)
        {
            if (i == size-1)
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
                    [i,&directory_file,&buffer,&errorOccurred](){
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
    catch (...) {
        {
            std::lock_guard<std::mutex> lock(mutex_cerr);
            std::cerr << "file " << directory_file << " is missing \n";
        }
        return std::make_shared<std::vector<std::string>>();
    }

}


void ReadFile::set_buffer(std::vector<std::string > &buffer,const size_t size_file,int maxThreads,size_t max_sizeMB)
{

    size_t numReadThreads = 1;

    numReadThreads = size_file / (max_sizeMB*1024*1024);


    if (numReadThreads > maxThreads )
        numReadThreads =  maxThreads;
    if (numReadThreads < 1) {
        numReadThreads = 1;
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

void ReadFile::readFileToBuffer(const std::string &directory_file, char *buffer, int start, int stop)
{

    std::ifstream input_file(directory_file, std::ios::binary);

    if (!input_file.is_open()) {
        throw std::runtime_error("File is missing: " + directory_file);
    } else
    {

        input_file.seekg(start, std::ios::beg);
        input_file.read(buffer, stop);
    }
}

//===================================================================================================================

ConverterJSON::ConverterJSON():list{std::make_shared<nlohmann::json>()}{


}

void ConverterJSON::filter_files(std::shared_ptr<nlohmann::json> filter_list,int str_size)
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

nlohmann::json ConverterJSON::parse_buffer(std::vector<std::string> &buffer)
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

void ConverterJSON::set_filter(int str_size, bool filter)
{
    if (str_size > 5)
        str_size = str_size;
    filter = filter;
    settingsChanged = true;
}

void ConverterJSON::set_filter(bool filter, int str_size)
{
    set_filter(str_size, filter);
}

void ConverterJSON::set_filter(int str_size)
{
    set_filter(str_size, filter);
}

void ConverterJSON::set_filter(bool filter)
{
    set_filter(str_size, filter);
}

void ConverterJSON::set_directory(std::string directory)
{
    this->directory = directory;
    time_reading = 0;
}

nlohmann::json ConverterJSON::reading_json(const std::string &directory_file,int maxThreads, size_t max_sizeMB )
{
    try {


        const size_t size_file = Info_file::size(directory_file);
        if (max_sizeMB *1024*1024 < size_file )
        {
            throw std::runtime_error("File size > " + std::to_string(size_file / 1024 / 1024) + " MB in " + directory_file);
        }
        auto buffer = std::make_shared<std::vector<std::string>>();
        buffer = ReadFile::readFile(directory_file,maxThreads,max_sizeMB);
        return parse_buffer(*buffer);
    }
    catch (const std::exception& e) {

        std::cerr << "error: " << e.what() << '\n';
        return nlohmann::json{};
    }
    catch (...) {
        return nlohmann::json{};
    }
}

//===================================================================================================================


int ConfigJSON::get_max_responses()
{
    return max_responses;
}

bool ConfigJSON::reading_config()
{
    try
    {     
        nlohmann::json json_data = reading_json(directory,maxThreads);
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

        time_reading = Info_file::data_sec(directory);
        *list = json_data["files"];
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

ConfigJSON::ConfigJSON(int maxThreads):ConverterJSON()
{
    this->maxThreads =  maxThreads;
    directory = "./config.json";
}

bool ConfigJSON::control_config(const nlohmann::json &json_data, const std::string &version, const std::string &project_name)
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
            throw std::runtime_error("version is incorrect -> " + json_data["config"]["version"].get<std::string>());
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

void ConfigJSON::update()
{
    try {
        if (time_reading < Info_file::data_sec(directory) && reading_config())
        {
            if(filter)
                filter_files(list,str_size);
        }
        else{
            if(filter)
                filter_files(list,str_size);
        }
    }
    catch (const std::exception& e) {

        std::cerr << "error: " << e.what() << '\n';

    }
    catch (...) {}
}

//===================================================================================================================

void RequestsJSON::update()
{
    try {
        if (time_reading < Info_file::data_sec(directory) )
        {
            nlohmann::json list_ = reading_json(directory,maxThreads);
            *list = list_["requests"];
            if(filter)
                filter_files(list,str_size);
        }
        else{
            if(filter)
                filter_files(list,str_size);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';

    }
    catch (...) {std::cerr << "error\n";}
}

RequestsJSON::RequestsJSON(int maxThreads):ConverterJSON()
{
    this->maxThreads =  maxThreads;
    directory = "./requests.json";
}



