#include "answersjson.h"
#include "converterjson.h"
#include <iostream>


Answers::Answers(std::shared_ptr<std::vector<RelativeIndex>> rel):rel{rel}
{
    directory = "./answers.json";
}

Answers::Answers()
{
    directory = "./answers.json";
}



bool Answers::get_answers(int max_responses)const
{
    if(rel != nullptr){
        std::sort(rel->begin(), rel->end(),[](RelativeIndex lhs, RelativeIndex rhs){
            return lhs.get_Relative_Relevancy() > rhs.get_Relative_Relevancy();});

        auto it = rel->begin();
        nlohmann::ordered_json js_;
        nlohmann::ordered_json js;

        js_["data"] = Info::get_time();

        for (int i{}; it !=rel->end() && i < max_responses; ++i,++it)
        {
            js[it->get_directory_file()] = it->get_Relative_Relevancy();
        }
        if(js.empty())
        {
            js_["answers"] = "false";

        }
        else js_["answers"] = js;

        return ConverterJSON::saveToFile(directory, js_);
    }
    else return false;
}


