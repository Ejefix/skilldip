#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json.hpp"
#include <thread>

namespace THReads {
inline const unsigned int num_threads{std::thread::hardware_concurrency()};

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
    ConverterJSON();
    virtual ~ConverterJSON(){};
    virtual void update() = 0;
    void set_filter(int str_size ,bool filter);
    void set_filter(bool filter,int str_size );
    void set_filter(int str_size );
    void set_filter(bool filter);
    void set_directory(std::string directory);
    std::shared_ptr<nlohmann::json> list;

    static nlohmann::json reading_json(const std::string &directory_file,int maxThreads = 1, size_t max_sizeMB = 300)  ; // примрено 100 MB
    int maxThreads;
protected:
    std::string directory;
    size_t time_reading{1};
    int str_size{300};
    bool filter{true};
    bool settingsChanged{false};
    // удаляет элемент, если не std::string или длинна выше  str_size
    static void filter_files(std::shared_ptr<nlohmann::json> list,int str_size);
private:
    // парсиниг от [0]->[1]-[2] ... , если отправили кривой буфер вернем пустой контейнер
    // буфер будет очищен
    static nlohmann::json parse_buffer(std::vector<std::string> &buffer) ;

};

class ConfigJSON : public ConverterJSON
{
public:
    // судя по тестам нету смысла больше 5
    ConfigJSON(int maxThreads = 1);

    void update() override;
    int get_max_responses();

private:

    // контроль шапки
    static bool control_config(const nlohmann::json &json_data,const std::string &version, const std::string &project_name);    
    bool reading_config();
    int max_responses{};

};

class RequestsJSON : public ConverterJSON
{
public:
    // судя по тестам нету смысла больше 5
    RequestsJSON(int maxThreads = 1);
    void update() override;
};
#endif // CONVERTERJSON_H
