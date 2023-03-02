#ifndef WEBSERVER_HTTP_RESPONSE_H
#define WEBSERVER_HTTP_RESPONSE_H
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<map>
#include<string>
#include"buffer.h"

class http_response {
private:
    std::string src_dir;
    std::string path;
    std::string method;
    int status_code;
    char* mmapFile;
    bool is_keepalive;
    struct stat file_status;
public:
    http_response();
    ~http_response();
    void init(std::string src_dir, std::string path, std::string method, int status_code, bool isKeepalive);
    static std::map<int,std::string> http_description;
    static std::map<int,std::string> failed_response_html_path;
    static std::map<std::string,std::string> content_type;
    std::string getfiletype();
    void addStateLine(buffer &buff);
    void addResponseHead(buffer &buff);
    void SetStatusCode();
    void mapFile();
    void unmapFile();
    void solveFailedSituations();
    void process(buffer& buff);
    char* get_mmapfile() const;
    struct stat get_filestatus() const;
};

#endif //WEBSERVER_HTTP_RESPONSE_H
