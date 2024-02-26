#include "readfile.h"
#include "converterjson.h"
#include <fstream>
#include <iostream>
#include <mutex>

namespace {
std::mutex mutex_cerr_;
}

std::shared_ptr<std::vector<std::string>> ReadFile::readFile(const std::string &directory_file,int maxThreads,  size_t max_sizeMB)
{

    try {
        if (maxThreads < 1 || maxThreads > THReads::num_threads)
            maxThreads = THReads::num_threads;

        size_t size_file = Info_file::size(directory_file);
        const auto buffer = std::make_shared<std::vector<std::string>>();

        set_buffer(*buffer, size_file,maxThreads,max_sizeMB );

        std::vector<std::thread> threads;

        size_t size = buffer->size();
        bool errorOccurred = false;

        for (int i{};i < size;++i)
        {
            if (i == size-1)
            {   try {

                    readFileToBuffer(directory_file, &((*buffer)[i][0]), buffer->at(0).size() * i, buffer->at(i).size());
                }
                catch (const std::exception& e) {
                    if (!errorOccurred)
                    {
                        errorOccurred = true;
                        std::lock_guard<std::mutex> lock(mutex_cerr_);
                        std::cerr << "error: " << e.what() << '\n';
                    }
                    buffer->at(i).clear();
                }
                catch (...) {

                    if (!errorOccurred)
                    {
                        errorOccurred = true;
                        std::lock_guard<std::mutex> lock(mutex_cerr_);
                        std::cerr << "error "  << '\n';
                    }
                    buffer->at(i).clear();
                }
            }
            else{
                threads.emplace_back(
                    [i,&directory_file,&buffer,&errorOccurred](){
                        try {
                            readFileToBuffer(directory_file, &((*buffer)[i][0]), buffer->at(0).size() * i, buffer->at(i).size());
                        }
                        catch (const std::exception& e) {
                            if (!errorOccurred)
                            {
                                errorOccurred = true;
                                std::lock_guard<std::mutex> lock(mutex_cerr_);
                                std::cerr << "error: " << e.what() << '\n';
                            }
                            buffer->at(i).clear();
                        }
                        catch (...) {

                            if (!errorOccurred)
                            {
                                errorOccurred = true;
                                std::lock_guard<std::mutex> lock(mutex_cerr_);
                                std::cerr << "error "  << '\n';
                            }
                            buffer->at(i).clear();
                        }
                    });}
        }
        for (auto& thread : threads) {
            thread.join();
        }
        return buffer;
    }
    catch (...) {
        {
            std::lock_guard<std::mutex> lock(mutex_cerr_);
            std::cerr << "file " << directory_file << " is missing \n";
        }
        return std::make_shared<std::vector<std::string>>();
    }

}

void ReadFile::set_buffer(std::vector<std::string > &buffer,const size_t size_file,int maxThreads,size_t max_sizeMB)
{

    size_t numReadThreads = 1;

    numReadThreads = size_file / (max_sizeMB*1024*1024);
    if (numReadThreads > maxThreads )
        numReadThreads =  maxThreads;
    if (numReadThreads < 1) {
        numReadThreads = 1;
    }
    size_t resize_string = size_file / numReadThreads;
    size_t remainder = size_file % numReadThreads;

    for (int i {}; i < numReadThreads - 1 ;++i)
    {
        std::string j;
        j.resize(resize_string);
        buffer.push_back(j);
    }
    std::string j;
    j.resize(resize_string + remainder);
    buffer.push_back(j);
}

void ReadFile::readFileToBuffer(const std::string &directory_file, char *buffer, int start, int stop)
{

    std::ifstream input_file(directory_file, std::ios::binary);

    if (!input_file.is_open()) {
        throw std::runtime_error("File is missing: " + directory_file);
    } else
    {

        input_file.seekg(start, std::ios::beg);
        input_file.read(buffer, stop);
    }
}

