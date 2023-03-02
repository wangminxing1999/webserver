#include "http_response.h"

std::map<int,std::string> http_response::http_description = {{200,"OK"}, {400, "Bad Request"}, {403, "Forbidden"}, {404, "Not Found"}};
std::map<int,std::string> http_response::failed_response_html_path = {{400, "/400.html"}, {403, "/403.html"}, {404, "/404.html"}};
std::map<std::string,std::string> http_response::content_type = {
        { ".html",  "text/html" },
        { ".xml",   "text/xml" },
        { ".xhtml", "application/xhtml+xml" },
        { ".txt",   "text/plain" },
        { ".rtf",   "application/rtf" },
        { ".pdf",   "application/pdf" },
        { ".word",  "application/nsword" },
        { ".png",   "image/png" },
        { ".gif",   "image/gif" },
        { ".jpg",   "image/jpeg" },
        { ".jpeg",  "image/jpeg" },
        { ".au",    "audio/basic" },
        { ".mpeg",  "video/mpeg" },
        { ".mpg",   "video/mpeg" },
        { ".avi",   "video/x-msvideo" },
        { ".gz",    "application/x-gzip" },
        { ".tar",   "application/x-tar" },
        { ".css",   "text/css "},
        { ".js",    "text/javascript "},
};

void http_response::SetStatusCode() {
    if(status_code == 400) return;
    int res = stat((src_dir+path).data(), &file_status);
    if(res == -1) {
        status_code = 404;
    } else if(!(file_status.st_mode & S_IROTH)){
        status_code = 403;
    } else {
        status_code = 200;
    }
}

void http_response::mapFile() {
    int fd = open((src_dir+path).data(), O_RDONLY);
    std::cout<<src_dir<<path<<std::endl;
    mmapFile = static_cast<char*>(mmap(0, static_cast<size_t>(file_status.st_size), PROT_READ, MAP_PRIVATE, fd, 0));
    close(fd);
}

void http_response::unmapFile() {
    munmap(mmapFile, static_cast<size_t>(file_status.st_size));
}

std::string http_response::getfiletype() {
    std::string::size_type index = path.find_last_of(".");
    if(index == std::string::npos) return "text/plain";
    if(content_type.find(path.substr(index)) != content_type.end()) {
        return content_type[path.substr(index)];
    }
    return "text/plain";
}

void http_response::addStateLine(buffer &buff) {
    buff.append("HTTP/1.1 " + std::to_string(status_code) + " " + http_description[status_code] + "\r\n");
    std::cout<<"add state line HTTP/1.1 "<<std::to_string(status_code)<<" "<<http_description[status_code]<<std::endl;
}

void http_response::addResponseHead(buffer &buff) {
    if(is_keepalive) {
        buff.append("Connection: keep-alive\r\n");
    } else {
        buff.append("Connection: close\r\n");
    }
    buff.append("Content-Type: " + getfiletype() + "\r\n");
    buff.append("Content-Length: " + std::to_string(file_status.st_size) + "\r\n\r\n");
}

void http_response::solveFailedSituations() {
    if(status_code == 200) return;
    bzero(&file_status, sizeof(file_status));
    path = failed_response_html_path[status_code];
    stat((src_dir+path).data(), &file_status);
}

struct stat http_response::get_filestatus() const {
    return file_status;
}

char *http_response::get_mmapfile() const {
    return mmapFile;
}

void http_response::init(std::string src_dir, std::string path, std::string method, int status_code, bool isKeepalive) {
    this->src_dir = src_dir;
    this->path = path;
    this->method = method;
    this->is_keepalive = isKeepalive;
    this->status_code = status_code;
}

http_response::http_response() {
    mmapFile = nullptr;
    file_status = {0};
    path = "";
    src_dir = "";
}

http_response::~http_response() {
    unmapFile();
}

void http_response::process(buffer& buff) {
    SetStatusCode();
    solveFailedSituations();
    addStateLine(buff);
    addResponseHead(buff);
    mapFile();
}
