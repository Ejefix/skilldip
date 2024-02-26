#ifndef READFILE_H
#define READFILE_H

#include <memory>
#include <string>
#include <vector>



class ReadFile  final
{
public:
    // функция читает файл бинарно и заполняет std::vector buffer от [0]->[1]->[2]->...
    static std::shared_ptr<std::vector<std::string>> readFile(const std::string &directory_file,int maxThreads = 1 ,  size_t max_sizeMB = 300);
private:
    // может включить дополнительные потоки для чтения
    static void set_buffer(std::vector<std::string> &buffer,const size_t size_file, int maxThreads , size_t max_sizeMB ) ;
    static  void readFileToBuffer(const std::string &directory_file ,char *buffer,int start,int stop);
};

#endif // READFILE_H
