#include <converterjson.h>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <thread>

// количество ядер
const unsigned int num_threads{ std::thread::hardware_concurrency()};
void start(ConverterJSON &x)
{

}


int main(int argc, char *argv[])
{
    int maxThreads = num_threads; // Значение по умолчанию


    try {

        if (argc > 1) {
            maxThreads = std::atoi(argv[1]); // Преобразуем аргумент командной строки в число
            if (maxThreads < 1 || maxThreads > num_threads)
                maxThreads = num_threads;
        }

        ConverterJSON x{maxThreads};
        start(x);
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
