#include <converterjson.h>
#include <iostream>
#include "searchserver.h"
#include "answersjson.h"

// git checkout

int main(int argc, char *argv[])
{
    int threads = 0;
    for(int i{1}; i < argc; ++i)
    {
        try {
            threads = std::stoi(argv[i]);
            std::cout << threads;

        } catch (const std::invalid_argument& e) {
        }
        break;
    }
    try {

        ConfigJSON x{};
        RequestsJSON y{};
        auto config = x.get_list();
        auto requests = y.get_list();
        if(config == nullptr && requests == nullptr)
        {
            if(!std::filesystem::exists("./answers.json") )
            {
                goto label;
            }
            std::cout << "There is no need to process the information;\nit is already up to date.\n";
        }
        else
        {           
        label:
            if (config == nullptr)
                config = x.get_list(true);
            if(requests == nullptr)
                requests = y.get_list(true);
            SearchServer search{config, requests, threads};
            Answers answer{search.get_RelativeIndex()};
            if(answer.get_answers(x.get_max_responses()))
            {
                std::cout << "Information updated\n";
                return 0;
            }
            else{
                std::cout << "Search completed, unable to find matches\n";
            }
        }
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr  << e.what() << '\n';
        return 1;
    }catch (...) {
        std::cerr << "main error! \n";
        return 2;
    }
}
