#include "searchserver.h"
#include <iostream>
#include <memory>
#include <fstream>
#include <functional>
#include <condition_variable>
#include <sstream>
#include <unordered_set>
#include "readfile.h"

namespace {
std::mutex insert_lock;
std::mutex mutex_cerr_;
}


size_t Dictionary::id = 1;

Dictionary::Dictionary()
{
    try {
        if(std::filesystem::exists(directory))
        {
            auto json = ConverterJSON::reading_json(directory);
            for (const auto& item : json.items())
            {
                std::string str = item.key();
                size_t value = item.value();
                push(str, value);
            }
        }
        else
        {
            if(!std::filesystem::exists("./dictionary"))
            {
                std::filesystem::create_directories("./dictionary");
            }
            else
            {
                std::filesystem::remove_all("./dictionary");
                std::filesystem::create_directories("./dictionary");
            }
        }
    }
    catch (const std::runtime_error& e) {        
        std::cerr << "error: Dictionary() \n" << e.what() << '\n';

    }
    catch (...) {        
        std::cerr << "error: Dictionary() "  << '\n';
        throw;
    }
}

Dictionary::~Dictionary()
{

}

bool Dictionary::insert(const std::string &str)
{
    std::lock_guard<std::mutex> lock(insert_lock);

    if (valueToID.find(str) != valueToID.end())
    {
        return false;
    }

    while (idToValue.find(id) != idToValue.end()) {
        ++id;
    }
    push(str, id);
    ++id;
    return true;
}

size_t Dictionary::at(const std::string &value)const
{
    return valueToID.at(value);
}

std::string Dictionary::at(size_t id)const
{
    return idToValue.at(id);
}

void Dictionary::push(const std::string &str, size_t value)
{
    valueToID[str] = value;
    idToValue[value] = str;
    if (value > id)
        id = value;
}

bool Dictionary::saveToFile() const
{
    nlohmann::json js;
    for (const auto& [value, id] : valueToID) {
        js[value] = id;
    }
    std::ofstream file(directory);
    if (!file.is_open()) {
        std::cerr << "File creation error " << directory << '\n';
        return false;
    }
    file << js;
    file.close();
    return true;
}

std::string Dictionary::get_directory() const
{
    return directory;
}

bool SearchServer::saveToFile(const std::string &directory_file, const std::map<size_t, size_t> &map) const
{
    size_t  size = sizeof(size_t);
    std::ofstream file(directory_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "File creation error " << directory_file << '\n';
        return false;
    }
    for (const auto &it : map) {

        file.write(reinterpret_cast<const char*>(&it.first), size);
        file.write(reinterpret_cast<const char*>(&it.second), size);
    }
    file.close();
    return true;
}

bool SearchServer::loadFromFile(const std::string &directory_file, std::map<size_t, size_t> &map) const
{
    size_t  size = sizeof(size_t);
    std::ifstream file(directory_file, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "File opening error " << directory_file << '\n';
        return false;
    }
    size_t key, value;
    while (file.read(reinterpret_cast<char*>(&key), size)
           && file.read(reinterpret_cast<char*>(&value), size))
    {
        map[key] = value;
    }
    file.close();
    return true;
}



SearchServer::SearchServer(const std::shared_ptr<const nlohmann::json>& config_files_list,
                           const std::shared_ptr<const nlohmann::json>& requests_list,int maxThreads)
    :config_files_list{config_files_list},requests_list{requests_list},
    relativeIndex{std::make_shared<std::vector<RelativeIndex>>()}
{

    SearchServerThreads = maxThreads;
    if (SearchServerThreads <  1 || SearchServerThreads > THReads::num_threads)
        SearchServerThreads = THReads::num_threads;

    if(!std::filesystem::exists("./dictionary"))
    {
        std::filesystem::create_directories("./dictionary");
    }
    RelativeIndex::max_relative = 0;
}

std::shared_ptr<std::vector<RelativeIndex>> SearchServer::get_RelativeIndex()
{
    if (config_files_list == nullptr || requests_list == nullptr)
    {
        std::cout << config_files_list << " " << requests_list << "\n";
        throw std::runtime_error{"class SearchServer -> Error list nullptr\n"};
    }
    if(config_files_list->empty() || requests_list->empty())
    {
        return std::make_shared<std::vector<RelativeIndex>>();
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
    dictionary.saveToFile();
    return relativeIndex;
}

std::shared_ptr<std::vector<std::map<size_t, size_t>>> SearchServer::get_result_files(const std::shared_ptr<const nlohmann::json> &config_files_list)
{
    std::vector<std::string> files_list;
    to_string(files_list, config_files_list);
    control_file(files_list);

    int size = files_list.size();

    std::vector<std::thread> threads;
    size_t counter{};

    std::mutex mutex;
    std::condition_variable cv;

    auto rezult = std::make_shared<std::vector<std::map<size_t, size_t>>>(size);

    for(size_t i {}; i < size; ++i)
    {
        if(files_list[i].empty()){
            continue;
        }
        try {
            std::hash<std::string> hasher;
            size_t word_hash = hasher(files_list[i]);
            std::string name_file =  "./dictionary/" + std::to_string(word_hash);
            if(control_read(files_list[i], name_file))
            {
                auto foo = [&, i, name_file](){
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        cv.wait(lock, [&] { return counter < SearchServerThreads; });
                        ++counter;
                    }
                    std::map<size_t, size_t> rez;
                    loadFromFile(name_file,rez);
                    auto it = rezult->begin();
                    *(it+i) = rez;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        --counter;
                    }
                    cv.notify_one();                  
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
                auto foo = [&, i, name_file](){
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        cv.wait(lock, [&] { return counter < SearchServerThreads; });
                        ++counter;
                    }
                    auto it = rezult->begin() + i ;
                    auto buffer =  ReadFile::readFile(files_list[i]);
                    *it = parse_buffer(buffer);
                    if(!it->empty())
                    {
                        saveToFile(name_file,*it);
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
        catch (const std::runtime_error& e) {
            std::lock_guard<std::mutex> lock( mutex_cerr_);
            std::cerr  << e.what() << '\n';
            continue;
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

size_t SearchServer::get_id(const std::string &word)
{

    dictionary.insert(word);
    return dictionary.at(word);
}

bool SearchServer::control_read(const std::string &directory_file, const std::string &json_file)
{
    if(directory_file.empty() )
        return false;

    size_t file;

    file = Info::time_file(directory_file);

    auto foo = [&json_file](){
        try{
            return Info::time_file(json_file);
        }
        catch (const std::runtime_error& e) {

            return size_t{};
        }
    };
    auto file_ = foo();
    return file_ > file;
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

void SearchServer::control_file(std::vector<std::string> &files_list) const
{
    std::unordered_set<std::string> files;
    for (auto &file : files_list)
    {
         if(!std::filesystem::exists(file)) {
            std::cerr << "File is missing: " + file << "\n";
            file.clear();
            continue;
         }
         if (!files.insert(file).second) {
            file.clear();
         }
    }
}

std::map<size_t, size_t> SearchServer::parse_buffer(const std::shared_ptr<std::vector<std::string>> buffer)
{
    try {
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
                    ++file[get_id(word_)];
                }
            }
        }
        buffer->clear();       
        return file;
    }
    catch (const std::runtime_error& e) {
        buffer->clear();
        std::lock_guard<std::mutex> lock( mutex_cerr_);
        std::cerr << "error: SearchServer::parse_buffer: " << e.what() << '\n';
        return std::map<size_t, size_t>();
    }
    catch (...) {
        buffer->clear();
        std::lock_guard<std::mutex> lock( mutex_cerr_);
        std::cerr << "error: SearchServer::parse_buffer: unknown error" << '\n';
        throw;
    }
}

std::vector<std::string> SearchServer::transformation(std::string &word) const
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
    if (word.size() > 0)
    {
        std::vector<std::string> words{word};
        return words;
    }
    return std::vector<std::string> {};
}
