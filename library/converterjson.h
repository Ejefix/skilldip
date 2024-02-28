#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json.hpp"
#include <thread>

namespace THReads {
inline const unsigned int num_threads{
                                      std::thread::hardware_concurrency() ?
        std::thread::hardware_concurrency() : 1};
}
struct Entry;
struct Entry2;

class Info_file final
{
public:
    static size_t data_sec (const std::string &directory_file);
    static size_t size(const std::string &directory_file);

};


class ConverterJSON
{
public:

    virtual ~ConverterJSON(){};

    //TEST
    friend void TestInvertedIndexFunctionality(
        const std::vector<std::string>& docs,
        const std::vector<std::string>& requests,
        const std::vector<std::vector<Entry>>& expected );
    friend void TestRelativeIndex(
        const std::vector<std::string>& docs,
        const std::vector<std::string>& requests,
        const std::vector<Entry2> &expected);

    std::shared_ptr<nlohmann::json> get_list();
    void set_directory(std::string directory);
    static nlohmann::json reading_json(const std::string &directory_file);

protected:
    virtual void parsing_list() = 0;
    bool update_list();
    std::shared_ptr<nlohmann::json> list;
    std::string directory;
private:
    size_t time_reading{1};
};

class ConfigJSON final : public ConverterJSON
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

class RequestsJSON final : public ConverterJSON
{
public:
    RequestsJSON();
private:
    void parsing_list() override;
};
#endif // CONVERTERJSON_H
