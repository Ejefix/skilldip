#include <converterjson.h>
#include <iostream>
#include "searchserver.h"


int main(int argc, char *argv[])
{
    try {

        ConfigJSON x{};
        RequestsJSON y{};
        SearchServer z{x.get_list(), y.get_list()};

        if(z.get_answers(x.get_max_responses()))
            std::cout << "Search completed successfully\n";
        else{std::cout << "Search completed, unable to find matches\n";}
        return 0;
    } catch (...) {
        std::cerr << "main error! \n";
    }
}
