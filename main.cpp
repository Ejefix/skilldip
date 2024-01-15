#include <converterjson.h>
#include <iostream>





int main(int argc, char *argv[])
{


    try {
        int numReadThreads = 1; // Значение по умолчанию
        if (argc > 1) {

            numReadThreads = std::atoi(argv[1]); // Преобразуем аргумент командной строки в число
        }
        ConverterJSON x{numReadThreads};
        if (!x.reading_config())
            return 1;
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
