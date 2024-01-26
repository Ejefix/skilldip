#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H
#include "converterjson.h"
#include <unordered_map>

class SearchServer: public Skeleton
{
public:
    SearchServer(int maxThreads,
                 const std::shared_ptr<const nlohmann::json>& config_files_list,
                 const std::shared_ptr<const nlohmann::json>& requests_list);
    ~SearchServer(){}


private:
    size_t SearchServerThreads;

    /* потоко безопастно, очистка буффера,
     first - это id из словаря;
     second - количество; */
    std::map<size_t, size_t> parse_buffer(const std::shared_ptr<std::vector<std::string>> buffer);

    // потоко безопастно
    size_t get_id(const std::string &word,std::vector<std::pair<std::string,size_t>> &miniBuffer);

    bool saveToFile(const std::string &directory_file, const std::map<size_t, size_t> &map) const;

    //вернем true если файл уже читался
    static bool control_read(const std::string &directory_file, const std::string &json_file );

     //результат чтения config.json
    const std::shared_ptr<const nlohmann::json> config_files_list;
    //результат чтения requests.json
    const std::shared_ptr<const nlohmann::json> requests_list;

    // переделает всё в символы
    void to_string(std::vector<std::string> &vec, const std::shared_ptr<const nlohmann::json> &list);

    //вернет результат с таким же индексом соответствующим config_files_list
    std::shared_ptr<std::vector<std::map<size_t, size_t>>> get_result_files(const std::shared_ptr<const nlohmann::json> &config_files_list);

    class Dictionary : public ConverterJSON
    {
    public:
        Dictionary();
        ~Dictionary();
        // потоко безопастно
        bool insert(const std::string &value);

        size_t at(const std::string &value)const;
        std::string at(size_t id)const;

    private:
        std::unordered_map<std::string, size_t> valueToID;
        std::unordered_map<size_t, std::string> idToValue;
        static size_t id;
        const std::string dictionary = "./work/dictionary.json";
        bool saveToFile() const;
    };
    Dictionary dictionary;
};

#endif // SEARCHSERVER_H
