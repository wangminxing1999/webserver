#ifndef WEBSERVER_HTTP_REQUEST_H
#define WEBSERVER_HTTP_REQUEST_H
#include<string>
#include<regex>
#include<iostream>
#include<map>
#include"buffer.h"

enum process_state {process_request_head, process_request_line, process_body};

class http_request {
private:
    process_state curr_state;
    std::string method, path, protocol;
    std::map<std::string,std::string> header;
public:
    http_request(){
        curr_state = process_request_head;
    }
    void init();
    virtual ~http_request(){}
    std::string getMethod() const;
    std::string getPath() const;
    std::string getProtocol() const;
    bool parse(buffer& buff);
    bool parse_request_head(const std::string& data);
    bool parse_request_line(const std::string& data);
    bool parse_body(const std::string& data);
    bool keepAlive() const;
};

#endif //WEBSERVER_HTTP_REQUEST_H
