#ifndef ANSWERSJSON_H
#define ANSWERSJSON_H
#include "relativeindex.h"
#include <memory>

class Answers final
{
public:
    Answers(std::shared_ptr<std::vector<RelativeIndex>> rel);
    Answers() = delete;
    bool get_answers(int max_responses)const;
     std::string directory;
private:    
    std::shared_ptr<std::vector<RelativeIndex>> rel;
};


#endif // ANSWERSJSON_H
