#include "searchserver.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <functional>
#include <condition_variable>



SearchServer::SearchServer(int maxThreads,
                           const std::shared_ptr<const nlohmann::json>& config_files_list,
                           const std::shared_ptr<const nlohmann::json>& requests_list)
    :Skeleton{1},config_files_list{config_files_list},requests_list{requests_list},
    relativeIndex{std::make_shared<std::vector<RelativeIndex>>()}
{
    //установка потоков
    SearchServerThreads = maxThreads;
    if (SearchServerThreads <  1 || SearchServerThreads > THReads::num_threads)
        SearchServerThreads = THReads::num_threads;

    if(!std::filesystem::exists("./work"))
    {
        std::filesystem::create_directories("./work");

    }
}

std::shared_ptr<std::vector<RelativeIndex>> SearchServer::get_RelativeIndex()
{
    if(config_files_list->empty() || config_files_list->empty())
    {
        return nullptr;
    }
    auto result_files = get_result_files(config_files_list);
    size_t i{};
    auto result_requests = get_result_requests(requests_list);
    for(auto &result : *result_files)
    {
        auto it = config_files_list->begin()+i;
        std::string directory_file = it->dump();
        int size = directory_file.size();
        if(size >0 && directory_file[0] == '"' && directory_file[size-1] == '"')
        {
            directory_file = directory_file.substr(1, size - 2);
        }
        std::vector<std::pair<std::string,size_t>> result_vec;


        for(auto &result_ : *result_requests)
        {
            std::pair<std::string,size_t> res;
            res.first = dictionary.at(result_.first);
            res.second = result[result_.first] ;
            if(res.second > 0 )
            {
                result_vec.push_back(res);
            }
        }
        if(!result_vec.empty() )
        {
            RelativeIndex ind{directory_file,result_vec};
            relativeIndex->push_back(ind);
        }
        ++i;
    }
     std::sort(relativeIndex->begin(), relativeIndex->end());
    return relativeIndex;
}

std::shared_ptr<std::vector<std::map<size_t, size_t>>> SearchServer::get_result_files(const std::shared_ptr<const nlohmann::json> &config_files_list)
{

    std::vector<std::string> files_list;


    to_string(files_list, config_files_list);
    int size = files_list.size();

    std::vector<std::thread> threads;
    size_t counter{};

    std::mutex mutex;
    std::condition_variable cv;

    auto rezult = std::make_shared<std::vector<std::map<size_t, size_t>>>(size);

    for(size_t i {}; i < size; ++i)
    {
        std::hash<std::string> hasher;
        size_t word_hash = hasher(files_list[i]);
        std::string json_file =  "./work/" + std::to_string(word_hash);

        bool control{false};
        for(size_t j {} ; j < i;++j)
        {
            if(files_list[i] == files_list[j])
            {
                control = true;
                break;
            }
        }
        if(control){

            continue;
        }
        if(control_read(files_list[i], json_file))
        {
            auto foo = [&, i, json_file](){
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock, [&] { return counter < SearchServerThreads; });
                    ++counter;
                }
                std::map<size_t, size_t> rez;
                auto json = dictionary.reading_json(json_file);
                for (const auto& item : json.items())
                {
                    std::string key = item.key();
                    std::stringstream sstream(key);
                    size_t key_ ;
                    sstream >>  key_;
                    size_t value = item.value();
                    rez[key_] = value;
                }
                auto it = rezult->begin();
                *(it+i) = rez;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    --counter;
                    cv.notify_one();
                }
            };
            if(SearchServerThreads == 1)
            {
                foo();
            }
            else
            {

                threads.emplace_back(foo);
            }
        }
        else
        {
            auto foo = [&, i, json_file](){
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock, [&] { return counter < SearchServerThreads; });
                    ++counter;
                }
                auto it = rezult->begin() + i ;
                auto buffer =  readFile(files_list[i]);
                *it = parse_buffer(buffer);
                if(!it->empty())
                {
                        saveToFile(json_file,*it);
                }
                {
                        std::unique_lock<std::mutex> lock(mutex);
                        --counter;
                        cv.notify_one();
                }
            };

            if(SearchServerThreads == 1)
            {
                foo();
            }
            else
            {
                threads.emplace_back(foo);
            }
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    return rezult;
}

std::shared_ptr<std::map<size_t, size_t> > SearchServer::get_result_requests(const std::shared_ptr<const nlohmann::json> &requests_list)
{
    auto requests = std::make_shared<std::map<size_t, size_t> >();

    auto buffer = std::make_shared<std::vector<std::string>>();
    to_string(*buffer, requests_list);
    *requests = parse_buffer(buffer);
    return requests;

}

size_t SearchServer::get_id(const std::string &word,std::map<std::string, size_t> &miniBuffer)
{

    auto it = miniBuffer.find(word);
    if (it != miniBuffer.end()) {
        return it->second;
    }

    dictionary.insert(word);
    size_t id = dictionary.at(word);
    miniBuffer[word] = id;

    return id;
}

bool SearchServer::control_read(const std::string &directory_file, const std::string &json_file)
{
    try {

         auto file = get_data_file_sec(directory_file);
         auto file_ = get_data_file_sec(json_file);
         return file_ > file;
    }
    catch (...)
    {
         return false;
    }
}

void SearchServer::to_string(std::vector<std::string> &vec, const std::shared_ptr<const nlohmann::json> &list)
{
    auto size = list->size();
    vec.reserve(size);
    for(auto &it:*list)
    {
         std::string stringValue = it.dump();

         if (stringValue.size() > 1 && stringValue.front() == '"' && stringValue.back() == '"') {
            stringValue = stringValue.substr(1, stringValue.size() - 2);
         }       
         vec.push_back(stringValue);
    }
}

std::map<size_t, size_t> SearchServer::parse_buffer(const std::shared_ptr<std::vector<std::string>> buffer)
{
    try {
        std::map<std::string, size_t> miniBuffer;
        std::map<size_t, size_t> file;
        for (auto &i : *buffer)
        {
            std::istringstream iss(i);
            std::string word;
            while (iss >> word) {
                std::vector<std::string> words;
                words = transformation(word);
                for(auto &word_ : words)
                {
                    auto id = get_id(word_,miniBuffer);
                    ++file[id];
                }
            }
        }
        buffer->clear();       
        return file;
    }
    catch (const std::exception& e) {
        buffer->clear();
        std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
        std::cerr << "error: SearchServer::parse_buffer: " << e.what() << '\n';
        return std::map<size_t, size_t>();
    }
    catch (...) {
        buffer->clear();
        std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
        std::cerr << "error: SearchServer::parse_buffer: unknown error" << '\n';
        return std::map<size_t, size_t>();
    }
}

std::vector<std::string> SearchServer::transformation(std::string &word) const  &
{
    word.erase(std::remove_if(word.begin(), word.end(), [](unsigned char c) {
                   return !(std::isalnum(static_cast<unsigned char>(c)) ||
                            c == '@' || c == '#' || c == '$' ||
                            c == '^' || c == '&' || c == '/' ||
                            c == '\\' || c == '%');
               }), word.end());
    std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    if (word.size() > 1)
    {
        std::vector<std::string> words{word};
        return words;
    }
    return std::vector<std::string> {};
}


size_t SearchServer::Dictionary::id = 1;

SearchServer::Dictionary::Dictionary():ConverterJSON{1}
{

    try {
        if(!std::filesystem::exists("./work"))
        {
            std::filesystem::create_directories("./work");

        }

        auto json = reading_json(dictionary);
       for (const auto& item : json.items())
        {
            std::string key = item.key();
            size_t value = item.value();
            valueToID[key] = value;
            idToValue[value] = key;
       }
    }
    catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
        std::cerr << "error: reading_json: " << e.what() << '\n';
    }
    catch (...) {
        std::lock_guard<std::mutex> lock(THReads::mutex_cerr);
        std::cerr << "error: reading_json: "  << '\n';
    }
}

SearchServer::Dictionary::~Dictionary()
{
    saveToFile();    
}

bool SearchServer::Dictionary::insert(const std::string &value)
{
    std::lock_guard<std::mutex> lock(THReads::insert_lock);

    if (valueToID.find(value) != valueToID.end())
    {
         return false;
    }

    while (idToValue.find(id) != idToValue.end()) {
         ++id;
    }
    valueToID[value] = id;
    idToValue[id] = value;
    ++id;
    return true;
}

size_t SearchServer::Dictionary::at(const std::string &value)const
{
    return valueToID.at(value);
}

std::string SearchServer::Dictionary::at(size_t id)const
{
    return idToValue.at(id);
}

bool SearchServer::Dictionary::saveToFile() const
{

    nlohmann::json js;
    for (const auto& [value, id] : valueToID) {
         js[value] = id;
    }
    std::ofstream file(dictionary.c_str());
    if (!file.is_open()) {
         std::cerr << "Не удалось открыть файл для записи: " << dictionary << '\n';
         return false;
    }
    file << js;
    file.close();
    return true;
}

bool SearchServer::saveToFile(const std::string &directory_file, const std::map<size_t, size_t> &map) const
{
    nlohmann::json js;
    for (const auto& [value, counter] : map) {
         std::string j = std::to_string(value);
         js[j] = counter;
    }
    std::ofstream file(directory_file.c_str());
    if (!file.is_open()) {
         std::cerr << "Не удалось открыть файл для записи: " << directory_file << '\n';
         return false;
    }
    file << js;
    file.close();
    return true;
}

