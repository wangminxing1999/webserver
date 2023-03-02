CXX=g++
CFLAGS=-std=c++11 -O2 -Wall -g
CXXFLAGS=-std=c++11 -O2 -Wall -g

TARGET:=myserver
SOURCE:=$(wildcard ../*.cpp)
OBJS=./buffer.cpp ./http_request.cpp ./http_response.cpp ./http_conn.cpp \
     ./timer.cpp ./epoller.cpp ./server.cpp ./main.cpp

$(TARGET):$(OBJS)
	$(CXX) $(CXXFLAGS)  $(OBJS) -o main -pthread
