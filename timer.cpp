#include"timer.h"

void TimeManager::swap(size_t a, size_t b) {
    std::swap(node_line[a], node_line[b]);
    record_location[node_line[a].fd] = a;
    record_location[node_line[b].fd] = b;
}

bool TimeManager::sift_down(size_t index, size_t n) {
    size_t father = index;
    size_t son = 2*father + 1;
    while(son < n) {
        if(son + 1 < n && node_line[son+1] < node_line[son]) son++;
        if(node_line[father] < node_line[son]) break;
        else {
            swap(father,son);
            father = son;
            son = 2*father + 1;
        }
    }
    return father > index;
}

void TimeManager::sift_up(size_t index) {
    int father = (index-1)/2;
    int son = index;
    while(father >= 0) {
        if(node_line[father] < node_line[son]) break;
        swap(father, son);
        son = father;
        father = (son-1)/2;
    }
}

void TimeManager::add_node(int fd, int timeout, const TimeCallBack &cb) {
    if(record_location.find(fd) == record_location.end()) {
        size_t insert_index = node_line.size();
        node_line.push_back({cb, MS(timeout) + Clock::now(), fd});
        record_location[fd] = insert_index;
        sift_up(insert_index);
    } else {
        size_t i = record_location[fd];
        node_line[i].expire_time = MS(timeout) + Clock::now();
        node_line[i].cb = cb;
        if(!sift_down(i, node_line.size())) {
            sift_up(i);
        }
    }
}

void TimeManager::del_node(size_t index) {
    size_t tail = node_line.size()-1;
    if(index < tail) {
        swap(tail, index);
        if(!sift_down(index,node_line.size())) {
            sift_up(index);
        }
    }
    record_location.erase(node_line.back().fd);
    node_line.pop_back();
}

void TimeManager::update(int id, int timeout) {
    if(record_location.find(id) == record_location.end()) return;
    size_t index = record_location[id];
    node_line[index].expire_time = Clock::now() + MS(timeout);
    sift_down(index,node_line.size());
}

void TimeManager::work(int fd) {
    if(record_location.find(fd) == record_location.end()) return;
    size_t index = record_location[fd];
    node_line[index].cb();
    del_node(index);
}

void TimeManager::handle_expire_events() {
    if(node_line.empty()) return;
    while(!node_line.empty()) {
        TimerNode node = node_line.front();
        if(std::chrono::duration_cast<MS>(node.expire_time - Clock::now()).count() <= 0) {
            node.cb();
            pop();
        } else {
            break;
        }
    }
}

void TimeManager::pop() {
    if(node_line.empty()) return;
    del_node(0);
}

int TimeManager::get_next_handle() {
    handle_expire_events();
    size_t res = -1;
    if(!node_line.empty()) {
        res = std::chrono::duration_cast<MS>(node_line.front().expire_time - Clock::now()).count();
        if(res < 0) res = 0;
    }
    return res;
}

void TimeManager::clear() {
    node_line.clear();
    record_location.clear();
}