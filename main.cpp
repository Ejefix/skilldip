#include <converterjson.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>
#include "searchserver.h"



int main(int argc, char *argv[])
{
    int maxThreads = -1; // Значение по умолчанию

    try {
        // Начало измерения времени
        auto start = std::chrono::high_resolution_clock::now();
        if (argc > 1) {
            maxThreads = std::atoi(argv[1]); // Преобразуем аргумент командной строки в число
          }

        ConverterJSON x{maxThreads};
        //  start(x);
        SearchServer y{maxThreads,x.config_files_list, x.requests_list};

        y.get_RelativeIndex();
        std::cout << y.get_data_file_sec("./work");
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            std::cout << "  Время выполнения: " << duration.count() << " мс" << std::endl;

        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
