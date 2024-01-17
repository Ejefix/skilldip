#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json_fwd.hpp"
class Skeleton
{
public:
    // maxThreads оставлено на усмотрение разработчика
    Skeleton(const int maxThreads );
    virtual ~Skeleton();
    static size_t get_data_file_sec (const std::string &directory_file);
    static const std::string main_project_version;
    static const std::string project_name;
    //принимаем в MB
    void set_max_size_PerThread(const float max_sizeMB);
protected:

    // это указания, что нужно написать как парсить буфер
    virtual void parse_buffer() = 0;

    // функция читает файл бинарно и заполняет std::vector от [0]->[1]->[2]->...
    std::shared_ptr<std::vector<std::string>> readFile(const std::string &directory_file,const size_t size_file)const;
    int maxThreads;
private:
    // может включить дополнительные потоки для чтения
    void set_buffer(std::vector<std::string> &buffer,const size_t size_file)const ;
    void readFileToBuffer(const std::string &directory_file ,char *buffer,int start,int stop)const;
    size_t max_sizeMB;
};


class ConverterJSON : public Skeleton
{
public:
    // судя по тестам нету смысла больше 5
    ConverterJSON(const int maxThreads = 5);

    nlohmann::json reading_json(const std::string &directory_file,const size_t max_file_size = 105000000)const  ; // примрено 100 MB

    void update_lists();

    // если нужны настройки фильтра
    std::shared_ptr<nlohmann::json> get_list_files_config(const int str_size = 300, const bool filter = true);
    std::shared_ptr<nlohmann::json> get_list_files_requests(const int str_size = 1000, const bool filter = true);
    std::shared_ptr<nlohmann::json> get_list_files_config(const bool filter);
    std::shared_ptr<nlohmann::json> get_list_files_requests(const bool filter);

    std::string config_directory = "config.json";
    std::string requests_directory = "requests.json";
    std::string answers_directory = "answers.json";

    //результат чтения config.json
    static std::shared_ptr<nlohmann::json> config_files_list;
    //результат чтения requests.json
    static std::shared_ptr<nlohmann::json> requests_list;


protected:

    void parse_buffer() override
        {}
private:
    size_t time_reading_config{1};
    size_t time_reading_requests{1};
    int str_size_config{300};
    int str_size_requests{1000};
    bool filter_config{true};
    bool filter_requests{true};

    int max_responses{};
    // контроль шапки
    static bool control_config(const nlohmann::json &json_data,const std::string &version, const std::string &project_name);

    // удаляет элемент, если не std::string или длинна выше const int str_size
    static void filter_files(std::shared_ptr<nlohmann::json> filter_list, const int str_size);

    // парсиниг от [0]->[1]-[2] ... , если отправили кривой буфер вернем пустой контейнер
    // буфер будет очищен
    nlohmann::json parse_buffer(std::vector<std::string> &buffer)const ;
    bool reading_config();
};

#endif // CONVERTERJSON_H
