#include <converterjson.h>
#include <iostream>





int main(int argc, char *argv[])
{

    ConverterJSON x{};
    if (!x.reading_config())
        return 1;
    std::cout << "ok " << '\n';
    return 0;
}
