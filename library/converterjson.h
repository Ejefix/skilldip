#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json.hpp"
#include <thread>

namespace THReads {
inline const unsigned int num_threads{
                                      std::thread::hardware_concurrency() ?
        std::thread::hardware_concurrency() : 1};
}

class Info_file final
{
public:
    static size_t data_sec (const std::string &directory_file);
    static size_t size(const std::string &directory_file);

};

class ReadFile  final
{
public:
    // функция читает файл бинарно и заполняет std::vector buffer от [0]->[1]->[2]->...
    static std::shared_ptr<std::vector<std::string>> readFile(const std::string &directory_file,int maxThreads = 1 ,  size_t max_sizeMB = 300);
private:
    // может включить дополнительные потоки для чтения
    static void set_buffer(std::vector<std::string> &buffer,const size_t size_file, int maxThreads , size_t max_sizeMB ) ;
    static  void readFileToBuffer(const std::string &directory_file ,char *buffer,int start,int stop);
};

class ConverterJSON
{
public:

    virtual ~ConverterJSON(){};

    std::shared_ptr<nlohmann::json> get_list();
    void set_directory(std::string directory);
    static nlohmann::json reading_json(const std::string &directory_file, size_t max_sizeMB = 100)  ; // примрено 100 MB

protected:
    virtual void parsing_list() = 0;
    bool update_list();
    std::shared_ptr<nlohmann::json> list;
    std::string directory;
private:
    size_t time_reading{1};

};

class ConfigJSON : public ConverterJSON
{
public:

    ConfigJSON();

    int get_max_responses();
    // удаляет элемент, если не std::string или длинна выше  str_size
    static void filter_str(std::shared_ptr<nlohmann::json> list,int str_size);

private:
    void parsing_list() override;
    // контроль шапки
    bool control_config(const std::string &version, const std::string &project_name);
    bool parsing_config();
    int max_responses{};

};

class RequestsJSON : public ConverterJSON
{
public:

    RequestsJSON();
private:

    void parsing_list() override;
};
#endif // CONVERTERJSON_H
