#include <converterjson.h>
#include <iostream>
#include "searchserver.h"



int main(int argc, char *argv[])
{
    int maxThreads = -1; // Значение по умолчанию
    try {

        ConverterJSON x{maxThreads};
        x.set_filter_configJSON(true);
        x.update();
        SearchServer y{maxThreads,x.config_files_list, x.requests_list};
        auto rel = y.get_RelativeIndex();
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
