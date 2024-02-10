#include "converterjson.h"
#include "searchserver.h"
#include <iostream>
#include <string>


void test_config(ConverterJSON &x,std::string &directory_file)
{

    x.config_directory = directory_file;
    x.update();
    if(x.config_files_list->empty())
    {
        std::cout << "test config ok - is incorrect \n";
    }
}
void test_count(std::shared_ptr<std::vector<RelativeIndex>> rel, std::string directory_file, std::string word, size_t result)
{

    size_t count {};
    for(auto it = rel->begin();it != rel->end();++it)
    {

        if (directory_file == it->get_directory_file())
        {
            auto vec = it->get_result();
            for (auto &vec_ : vec) {
                if(vec_.first == word )
                {
                    count = vec_.second;
                }
            }
        }
    }
    if ( count == result)
    {
        std::cout << "test count ok\n";
    }
    else
    {
        std::cout << directory_file << " test count not ok " << count << " vs " << result << '\n';
    }
}
void test_timeRequests(std::string directory_file)
{
    auto start = std::chrono::high_resolution_clock::now();
    int maxThreads = -1;
    ConverterJSON x{maxThreads};
    x.config_directory = "./tests/file/config.json";
    x.requests_directory = directory_file;
    x.update();

    SearchServer y{maxThreads,x.config_files_list, x.requests_list};
    auto rel = y.get_RelativeIndex();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "test time Requests : " << duration.count() << " ms" << std::endl;
}

int main(int argc, char **argv) {
    int maxThreads = -1;
    try {

        auto start = std::chrono::high_resolution_clock::now();
        std::string directory_file[3] = {"./tests/file/config1.json",
                                         "./tests/file/config2.json",
                                         "./tests/file/config.json"};
        int maxThreads = -1;
        ConverterJSON x{maxThreads};
        x.requests_directory = "./tests/file/requests.json";
        test_config(x,directory_file[0]);
        test_config(x,directory_file[1]);
        test_config(x,directory_file[2]);
        SearchServer y{maxThreads,x.config_files_list, x.requests_list};
        auto rel = y.get_RelativeIndex();
        test_count(rel, "./tests/file/Biggers2.txt", "test2",300);
        test_count(rel, "./tests/file/Biggers2.txt", "test",200);
        test_count(rel, "./tests/file/Biggers2.txt", "test", 50);


        test_timeRequests("./tests/file/requests.json");
        std::cout << "standart requests.json\n";


        test_timeRequests("./tests/file/requests2.json");
        std::cout << "standart requests.json x5 \n";

        test_timeRequests("./tests/file/requests3.json");
        std::cout << "standart requests.json x25 \n";


        test_timeRequests("./tests/file/requests4.json");
        std::cout << "standart requests.json x40 \n\n";

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout << "full tests time : " << duration.count() << " ms" << std::endl;


        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
