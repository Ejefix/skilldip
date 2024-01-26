#include <converterjson.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include "searchserver.h"

void start(ConverterJSON &x)
{
    std::cout <<  x.config_files_list->dump(4);
    std::cout <<  x.requests_list->dump(4);
}


int main(int argc, char *argv[])
{
    int maxThreads = -1; // Значение по умолчанию


    try {

        if (argc > 1) {
            maxThreads = std::atoi(argv[1]); // Преобразуем аргумент командной строки в число
          }

        ConverterJSON x{maxThreads};
        //  start(x);
        SearchServer y{maxThreads,x.config_files_list, x.requests_list};

        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
