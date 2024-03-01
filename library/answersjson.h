#ifndef ANSWERSJSON_H
#define ANSWERSJSON_H
#include "relativeindex.h"
#include <memory>



class Answers final
{
public:
    Answers(std::shared_ptr<std::vector<RelativeIndex>> rel);
    Answers();
    bool get_answers(int max_responses)const;
private:
    void get_rel();
    std::shared_ptr<std::vector<RelativeIndex>> rel;
    std::string directory;
};


#endif // ANSWERSJSON_H
