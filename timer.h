#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H
#include<ctime>
#include<chrono>
#include<functional>
#include<memory>
#include<vector>
#include<unordered_map>
typedef std::function<void()> TimeCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

class TimerNode {
public:
    TimeCallBack cb;
    TimeStamp expire_time;
    int fd;

    bool operator <(const TimerNode& timenode) {
        return expire_time < timenode.expire_time;
    }
};

class TimeManager {
private:
    std::vector<TimerNode> node_line;
    std::unordered_map<int,size_t> record_location;
    void sift_up(size_t index);
    bool sift_down(size_t index, size_t n);
    void swap(size_t a, size_t b);
public:
    TimeManager() {
        node_line.reserve(64);
    }
    ~TimeManager() {
        clear();
    }
    void add_node(int fd, int timeout, const TimeCallBack& cb);
    void del_node(size_t index);
    void work(int fd);
    int get_next_handle();
    void handle_timeout();
    void update(int id, int timeout);
    void handle_expire_events();
    void pop();
    void clear();
};

#endif //WEBSERVER_TIMER_H
