#include <converterjson.h>
#include <iostream>
//#include <memory>
//#include <nlohmann/json.hpp>

void start(ConverterJSON &x)
{
    std::shared_ptr<nlohmann::json> config_files_list{x.get_list_files_config()};

   // std::cout << config_files_list->dump(4) << std::endl;

    std::shared_ptr<nlohmann::json>  list{x.get_list_files_requests()};
}


int main(int argc, char *argv[])
{
    try {
        int numReadThreads = 1; // Значение по умолчанию
        if (argc > 1) {
            numReadThreads = std::atoi(argv[1]); // Преобразуем аргумент командной строки в число
        }
        ConverterJSON x{numReadThreads};
        start(x);
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
