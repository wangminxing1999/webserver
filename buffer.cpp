#include"buffer.h"

char* buffer::begin_ptr() {
    return &*buff.begin();
}

std::size_t buffer::writable_size() {
    return buff.size() - write_index;
}

std::size_t buffer::readable_size() {
    return write_index - read_index;
}

void buffer::allocate_space(std::size_t more_size) {
    buff.resize(buff.size() + more_size + 1);
}

void buffer::ensure_writable(std::size_t size) {
    if(size > writable_size()) {
        allocate_space(size - writable_size());
    }
}

ssize_t buffer::read(int fd, int* Errno) {
    char extra_buf[65536];
    struct iovec iv1, iv2;
    struct iovec ivs[2];
    iv1.iov_base = begin_ptr() + write_index;
    iv1.iov_len = writable_size();
    iv2.iov_base = extra_buf;
    iv2.iov_len = 65536;
    ivs[0] = iv1;
    ivs[1] = iv2;
    ssize_t recv_data = readv(fd, ivs, 2);
    if(recv_data < 0) {
        std::cout<<"从socket中读取数据失败!\n"<<std::endl;
        *Errno = errno;
        return recv_data;
    }
    if(static_cast<size_t>(recv_data) <= writable_size()) {
        write_index += static_cast<size_t>(recv_data);
    } else {
        write_index = buff.size();
        ensure_writable(static_cast<size_t>(recv_data));
        for(size_t i = 0; i < static_cast<size_t>(recv_data) - writable_size(); i++) {
            buff[write_index] = extra_buf[static_cast<int>(i)];
            write_index++;
        }
    }
    return recv_data;
}

ssize_t buffer::writeToBuffer(int fd, int* Errno) {
    ssize_t write_data = write(fd, begin_ptr()+read_index, readable_size());
    if(write_data < 0) {
        std::cout<<"向socket写入数据失败!\n"<<std::endl;
        *Errno = errno;
        return write_data;
    } else {
        read_index += static_cast<size_t>(write_data);
    }
    return write_data;
}

void buffer::initPtr() {
    bzero(&buff[0], buff.size());
    read_index = 0;
    write_index = 0;
}

void buffer::updateReadIndex(int n) {
    read_index += n;
}

void buffer::updateWriteIndex(int n) {
    write_index += n;
}

void buffer::append(const std::string &s) {
    ensure_writable(s.size());
    for(int i = 0; i < s.size(); i++) {
        buff[write_index] = s[i];
        write_index++;
    }
}

int buffer::getReadIndex() const {
    return read_index;
}

int buffer::getWriteIndex() const {
    return write_index;
}
