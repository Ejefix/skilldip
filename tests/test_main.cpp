#include "converterjson.h"
#include "searchserver.h"
#include <iostream>
#include <string>

int maxThreads = 0;
int test{0};
std::vector<std::pair<double,int>> times_;

bool test_config(ConfigJSON &x,std::string directory_file)
{
    x.set_directory( directory_file);
    if(x.get_list()->empty())
    {
        return false;
    }
    return true;
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
                    break;
                }
            }
        }
    }
    if ( count == result)
    {
        std::cout << "test count ok -> " << count << " vs " << result << '\n';;
    }
    else
    {
        std::cout << directory_file << " test count not ok -> " << count << " vs " << result << '\n';
    }
}
void test_timeRequests(std::string directory_file)
{
    auto start = std::chrono::high_resolution_clock::now();
    ConfigJSON x{};
    x.set_directory("./tests/file/config.json");
    RequestsJSON y{};
    y.set_directory(directory_file);
    SearchServer z{x.get_list(), y.get_list(),maxThreads};
    auto rel = z.get_RelativeIndex();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "test time Requests : " << duration.count() << " ms" << std::endl;
}

int main(int argc, char *argv[]) {
    int th = THReads::num_threads;
    std::pair<double,int> count{99999,99999};
    for(int i{1}; i < argc; ++i) {
        if (strcmp(argv[i], "T1") == 0) {
            th = 0;
            break;
        }
        if (strcmp(argv[i], "T2") == 0) {
            test = 1;
            maxThreads = 1;
            continue;
        }
        if (i == 2)
        {
             try {
                th = std::stoi(argv[i]);
                if(th > THReads::num_threads)
                    th = THReads::num_threads;
             } catch (const std::invalid_argument& e) {
                th = THReads::num_threads;
             }
            break;
        }
    }


    try {

        while (test <= th){

            auto start = std::chrono::high_resolution_clock::now();

            if(test == 0){
                ConfigJSON x{};
                RequestsJSON y{};
                y.set_directory("./tests/file/requesssts.json");
                y.set_directory("./tests/file/requests.json");

                std::cout  << "test config start \n" << (!test_config(x, "./tests/file/config1.json") ? "test true " : "test false") << "\n\n";
                std::cout  << "test config start \n" << (!test_config(x, "./tests/file/config2.json") ? "test true" : "test false")<< "\n\n";
                std::cout  << "test config start \n" << (!test_config(x, "./tests/file/confssig.json") ? "test true" : "test false")<< "\n\n";
                std::cout  << "test config start \n" << (test_config(x, "./tests/file/config.json") ? "test true" : "test false")<< "\n\n";


                test_timeRequests("./tests/file/requests.json");
                std::cout << "standart requests.json \n\n";


                test_timeRequests("./tests/file/requests2.json");
                std::cout << "standart requests.json x5 \n\n";

                test_timeRequests("./tests/file/requests3.json");
                std::cout << "standart requests.json x25 \n\n";

                test_timeRequests("./tests/file/requests4.json");
                std::cout << "standart requests.json x40 \n\n";


                SearchServer z{x.get_list(), y.get_list()};

                auto  rel = z.get_RelativeIndex();

                test_count(rel, "./tests/file/Biggers2.txt", "test2",300);
                test_count(rel, "./tests/file/Biggers2.txt", "test",200);
                test_count(rel, "./tests/file/Biggers2.txt", "test4", 0);

                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;
                std::cout << "full tests time : " << duration.count() << " ms" << "\n\n";
            }
            else
            {
                std::cout << "wait, thread test " << maxThreads << '\n';
                ConfigJSON x{};
                RequestsJSON y{};
                x.set_directory("./tests/file/config4.json");
                y.set_directory("./tests/file/requests4.json");

                SearchServer z{x.get_list(), y.get_list(),maxThreads};
                auto rel = z.get_RelativeIndex();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end - start;

                times_.push_back(std::make_pair(duration.count(),maxThreads));
            }
            ++test;
            ++maxThreads;
        }
        for(auto &j:times_)
        {
            if (count.first > j.first)
            {
                count = j;
            }
            std::cout << "full  time : " << j.first << " ms, " ;
            std::cout << " maxThreads : " << j.second  << "\n";
        }
        std::cout << "\n minimum  time : \n"  ;
        std::cout << "full  time : " << count.first << " ms, " ;
        std::cout << " maxThreads : " << count.second  << "\n";
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
