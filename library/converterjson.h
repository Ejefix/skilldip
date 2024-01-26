#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json.hpp"
#include <mutex>
#include <thread>

namespace THReads {
inline const unsigned int num_threads{std::thread::hardware_concurrency()};
 extern std::mutex mutex_cerr;
extern std::mutex insert_lock;
}
class Skeleton
{
public:
    // maxThreads оставлено на усмотрение разработчика
    Skeleton(int maxThreads );
    virtual ~Skeleton();
    static size_t get_data_file_sec (const std::string &directory_file);
    static size_t get_file_size(const std::string &directory_file);
    static const std::string main_project_version;
    static const std::string project_name;
    //принимаем в MB
    void set_max_size_PerThread(float max_sizeMB);
protected:

    // функция читает файл бинарно и заполняет std::vector buffer от [0]->[1]->[2]->...
    std::shared_ptr<std::vector<std::string>> readFile(const std::string &directory_file)const;
    int maxThreads;
    float max_sizeMB;
private:

    // может включить дополнительные потоки для чтения
    void set_buffer(std::vector<std::string> &buffer,const size_t size_file)const ;
    void readFileToBuffer(const std::string &directory_file ,char *buffer,int start,int stop)const;
};


class ConverterJSON : public Skeleton
{
public:
    // судя по тестам нету смысла больше 5
    ConverterJSON(int maxThreads);

    nlohmann::json reading_json(const std::string &directory_file,const size_t max_file_size = 105000000)const  ; // примрено 100 MB

    void update();

    void set_filter_configJSON(int str_size ,bool filter);
    void set_filter_configJSON(int str_size );
    void set_filter_configJSON(bool filter);
    void set_filter_requestsJSON(int str_size , bool filter);
    void set_filter_requestsJSON(int str_size );
    void set_filter_requestsJSON(bool filter);


    std::string config_directory = "./config.json";
    std::string requests_directory = "./requests.json";
    std::string answers_directory = "./answers.json";

    //результат чтения config.json
const     std::shared_ptr<nlohmann::json> config_files_list;
    //результат чтения requests.json
const     std::shared_ptr<nlohmann::json> requests_list;


private:
    size_t time_reading_config{1};
    size_t time_reading_requests{1};

    int str_size_config{300};
    int str_size_requests{1000};
    bool filter_config{false};
    bool filter_requests{false};
    bool settingsChanged{false};
    int max_responses{};
    // контроль шапки
    static bool control_config(const nlohmann::json &json_data,const std::string &version, const std::string &project_name);

    // удаляет элемент, если не std::string или длинна выше int str_size
    static void filter_files(std::shared_ptr<nlohmann::json> filter_list,int str_size);

    // парсиниг от [0]->[1]-[2] ... , если отправили кривой буфер вернем пустой контейнер
    // буфер будет очищен
    nlohmann::json parse_buffer(std::vector<std::string> &buffer)const ;
    bool reading_config();

    void get_list_files_config(int str_size , const bool filter);
    void get_list_files_requests(int str_size, const bool filter);
};

#endif // CONVERTERJSON_H
