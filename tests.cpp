#include "gtest/gtest.h"
#include "relativeindex.h"
#include <string>
#include <vector>
#include <converterjson.h>
#include <iostream>
#include "searchserver.h"
#include <fstream>


void WriteTextToFile(const std::string& filename, const std::string& text) {

    std::ofstream file(filename);
    if (!file.is_open()) {

        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
            return;
    }
    file << text;
    file.close();
}

struct Entry {
    std::string doc_id;
    size_t count;

    Entry(std::string doc_id, size_t count) : doc_id(doc_id), count(count) {}
    bool operator ==(const Entry& other) const {
        return (doc_id == other.doc_id &&
                count == other.count);
    }

};
void TestInvertedIndexFunctionality(
    const std::vector<std::string>& docs,
    const std::vector<std::string>& requests,
    const std::vector<std::vector<Entry>>& expected )
{
    ConfigJSON x{};
    x.list = std::make_shared<nlohmann::json> (docs);
    RequestsJSON y{};
    y.list = std::make_shared<nlohmann::json> (requests);


    SearchServer z{x.list,y.list};
    auto rel = z.get_RelativeIndex();


    std::vector<std::vector<Entry>> result;
    std::vector<Entry> result_;
    for (auto &request : requests)
    {
        for (auto it = rel->begin(); it != rel->end(); ++it)
        {
            auto doc_id = it->get_directory_file();
            auto vec_pair = it->get_result();
            for (auto &vec_pair_ : vec_pair)
            {
                if (request == vec_pair_.first)
                {
                   // std::cout << it->get_directory_file() << " " << vec_pair_.first << " = " << vec_pair_.second << '\n';
                    Entry z{it->get_directory_file(),vec_pair_.second};
                    result_.push_back(z);
                }
            }
        }
        result.push_back(result_);
        result_.clear();
    }
    ASSERT_EQ(result, expected);
}

TEST(InvertedIndexTest, Functionality) {


    std::vector<std::string>  docs = {"./tests/file/test1.txt",
                                     "./tests/file/test2.txt",
                                     "./tests/file/test3.txt"};

    const std::vector<std::string>  requests = {"text", "111", "222", "333"};
    WriteTextToFile(docs[0], requests[0] + " " + requests[1]);
    WriteTextToFile(docs[1], requests[0] + " " + requests[2]);
    WriteTextToFile(docs[2], requests[0] + " " + requests[3]);

    const std::vector<std::vector<Entry>> expected = {
        {
         Entry(docs[0], 1), Entry(docs[1], 1), Entry(docs[2], 1)
        },
        {Entry(docs[0], 1)}, // "111" встречается только в первом документе
        {Entry(docs[1], 1)}, // "222" - во втором
        {Entry(docs[2], 1)}  // "333" - в третьем
    };

    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestBasic2)
{

    std::vector<std::string>  docs = {"./tests/file/test1.txt",
                                     "./tests/file/test2.txt",
                                     "./tests/file/test3.txt",
                                     "./tests/file/test4.txt"};

    WriteTextToFile(docs[0], "milk milk milk milk water water water");
    WriteTextToFile(docs[1], "milk water water");
    WriteTextToFile(docs[2], "milk milk milk milk milk water water water water water");
    WriteTextToFile(docs[3], "americano cappuchino");
    const std::vector<std::string> requests = {"milk", "water", "cappuchino"};
    const std::vector<std::vector<Entry>> expected = {
        {
         Entry{docs[0], 4}, Entry{docs[1], 1}, Entry{docs[2], 5}
        },
        {
            Entry{docs[0], 3}, Entry{docs[1], 2}, Entry{docs[2], 5}
        },
        {
            Entry{docs[3], 1}
        }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}
TEST(TestCaseInvertedIndex, TestInvertedIndexMissingWord) {


    std::vector<std::string>  docs = {"./tests/file/test1.txt",
                                     "./tests/file/test2.txt"};

    WriteTextToFile(docs[0], "a b c d e f g h i j k l");
    WriteTextToFile(docs[1], "statement");
    const std::vector<std::string> requests = {"m", "statement"};
    std::vector<std::vector<Entry>> expected = {
        {},
        { Entry{docs[1], 1}}
    } ;
    TestInvertedIndexFunctionality(docs, requests, expected);
}
