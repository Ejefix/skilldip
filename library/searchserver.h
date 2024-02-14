#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H
#include "converterjson.h"
#include "relativeindex.h"

class SearchServer
{
public:
    SearchServer(int maxThreads,
                 const std::shared_ptr<const nlohmann::json>& config_files_list,
                 const std::shared_ptr<const nlohmann::json>& requests_list);

    virtual ~SearchServer(){};
    std::shared_ptr<std::vector<RelativeIndex>> get_RelativeIndex();
    bool get_answers(int max_responses, const std::string &directory_file = "./answers.json");

    // возможно захотим распарсить ссылки или есть другие требования
    // а может хотите искать константное слово, учитывая регистр и другие факторы
    // часть логики parse_buffer
    virtual std::vector<std::string> transformation(std::string &word) const ;
private:
    std::shared_ptr<std::vector<RelativeIndex>> relativeIndex;
    size_t SearchServerThreads;

    /* потоко безопастно, очистка буффера,
     first - это id из словаря;
     second - количество; */
    std::map<size_t, size_t> parse_buffer(const std::shared_ptr<std::vector<std::string>> buffer);

    // потоко безопастно
    size_t get_id(const std::string &word,std::map<std::string, size_t> &miniBuffer);

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
    std::shared_ptr<std::map<size_t, size_t>> get_result_requests(const std::shared_ptr<const nlohmann::json> &requests_list) ;
    class Dictionary
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
        std::string directory = "./work/dictionary.json";
        bool saveToFile() const;
    };
    Dictionary dictionary;
};

#endif // SEARCHSERVER_H
