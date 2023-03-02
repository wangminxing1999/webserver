#include"http_request.h"

std::string http_request::getMethod() const {
    return method;
}

std::string http_request::getPath() const {
    return path;
}

std::string http_request::getProtocol() const {
    return protocol;
}

bool http_request::parse(buffer& buff) {
    std::cout<<"parse"<<std::endl;
    std::string data = std::string(buff.begin_ptr()+buff.getReadIndex(), buff.begin_ptr()+buff.getWriteIndex());
    std::regex reg(".*\r\n");
    std::smatch results;
    std::string::const_iterator iterStart = data.begin();
    std::string::const_iterator iterEnd = data.end();
    std::string temp;
    while(std::regex_search(iterStart, iterEnd, results, reg)) {
        temp = results[0];
        if(curr_state == process_request_head) {
            bool success = parse_request_head(temp);
            if(!success) {
                std::cout<<"解析请求头失败！\n"<<std::endl;
                return false;
            }
        } else if(curr_state == process_request_line) {
            bool success = parse_request_line(temp);
            if(!success) {
                std::cout<<"解析请求行失败！\n"<<std::endl;
                return false;
            }
        } else if(curr_state == process_body) {
            return true;
        }
        iterStart = results[0].second;
    }
    return true;
}

bool http_request::parse_request_head(const std::string& data) {
    std::regex reg("[\\S]*[\\S]");
    std::smatch results;
    std::string::const_iterator iterStart = data.begin();
    std::string::const_iterator iterEnd = data.end();
    std::string temp;
    int count = 1;
    while(std::regex_search(iterStart, iterEnd, results, reg)) {
        temp = results[0];
        std::cout<<temp<<std::endl;
        switch (count) {
            case 1:
                method = temp;
            case 2:
                path = temp;
            case 3:
                protocol = temp;
        }
        iterStart = results[0].second;
        count++;
    }
    if(count != 4) return false;
    curr_state = process_request_line;
    return true;
}

bool http_request::parse_request_line(const std::string& data) {
    std::cout<<"start parse_request_line\r\n"<<std::endl;
    if(data == "\r\n") {
        curr_state = process_body;
        return true;
    }
    std::regex reg("^([^:]*): ([^:]*)$");
    std::smatch results;
    std::string::const_iterator iterStart = data.begin();
    std::string::const_iterator iterEnd = data.end();
    std::string key, value;
    if(std::regex_search(iterStart, iterEnd, results, reg)) {
        key = results[1];
        value = results[2];
        header[key] = value;
        std::cout<<key<<": "<<value<<std::endl;
    }
    return true;
}

bool http_request::parse_body(const std::string& data) {
    return true;
}

bool http_request::keepAlive() const {
    if(header.find("Connection")->second == "keep-alive" && protocol == "HTTP/1.1") {
        return true;
    }
    return false;
}

void http_request::init() {
    curr_state = process_request_head;
}