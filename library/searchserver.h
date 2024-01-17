#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H
#include "nlohmann/json_fwd.hpp"

class SearchServer
{
public:
    SearchServer(const std::shared_ptr<nlohmann::json> config_files_list,
                 const std::shared_ptr<nlohmann::json> requests_list);




    //результат чтения config.json
    std::shared_ptr<nlohmann::json> config_files_list;
    //результат чтения requests.json
    std::shared_ptr<nlohmann::json> requests_list;

};

#endif // SEARCHSERVER_H
