#ifndef CONVERTERJSON_H
#define CONVERTERJSON_H
#include "nlohmann/json.hpp"
#include <thread>
#include "relativeindex.h"

namespace THReads {
inline const unsigned int num_threads{
                                      std::thread::hardware_concurrency() ?
        std::thread::hardware_concurrency() : 1};
}
struct Entry;
struct Entry2;

class Info final
{
public:
    // @ return time seconds
    static size_t time_file (const std::string &directory_file);

    static size_t size_file(const std::string &directory_file);

    // @ return "%Y-%m-%d %H:%M:%S" current time
    static std::string get_time();
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

    std::shared_ptr<nlohmann::json> get_list(bool forcibly = false);
    virtual void set_directory(const std::string &directory);
    std::string get_directory();
    static nlohmann::json reading_json(const std::string &directory_file);
    static bool saveToFile(const std::string &directory_file, const nlohmann::ordered_json &js);
protected:

    virtual bool parsing_list(bool forcibly) = 0;
    virtual bool update_list(bool forcibly);
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
    void set_directory(const std::string &directory)override;
    // удаляет элемент, если не std::string или длинна выше  str_size
    static void filter_str(std::shared_ptr<nlohmann::json> list,int str_size);

private:

    // контроль шапки
    bool control_config(const std::string &version, const std::string &project_name);
    bool parsing_config();

    bool parsing_list(bool forcibly) override;
    bool update_list(bool forcibly) override;

    int max_responses{};
    std::string info_file = "./js/config.brb";
};

class RequestsJSON final : public ConverterJSON
{
public:
    RequestsJSON();
    void set_directory(const std::string &directory)override;
private:
    bool parsing_list(bool forcibly) override;
    bool update_list(bool forcibly) override;
    std::string info_file = "./js/requests.brb";
};





#endif // CONVERTERJSON_H
