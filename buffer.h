#ifndef WEBSERVER_BUFFER_H
#define WEBSERVER_BUFFER_H
#define INITIAL_BUFFER_SIZE 512
#include<stdio.h>
#include<cstring>
#include<vector>
#include<cstddef>
#include<unistd.h>
#include<atomic>
#include<errno.h>
#include<sys/uio.h>
#include<iostream>
class buffer {
private:
    std::vector<char> buff;
    std::atomic<std::size_t> read_index;
    std::atomic<std::size_t> write_index;
public:
    buffer() {
        buff = std::vector<char>(INITIAL_BUFFER_SIZE);
        read_index = 0;
        write_index = 0;
    }
    std::size_t writable_size();
    std::size_t readable_size();
    int getReadIndex() const;
    int getWriteIndex() const;
    char* begin_ptr();
    void allocate_space(std::size_t);
    void ensure_writable(std::size_t size);
    void initPtr();
    void updateReadIndex(int n);
    void updateWriteIndex(int n);
    void append(const std::string& s);
    ssize_t read(int fd, int* Errno);//与客户端I/O进行交互
    ssize_t writeToBuffer(int fd, int* Errno);
};

#endif //WEBSERVER_BUFFER_H
