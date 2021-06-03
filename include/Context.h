//
// Created by xj on 12/10/20.
//

#ifndef MY_HOST_LINUX_CONTEXT_H
#define MY_HOST_LINUX_CONTEXT_H
#include "common.h"
#include <atomic>

class Context
{
public:
    int num_of_streams;
    int packet_cnt;
    char* master_path;

public:
    void init(int packet_cnt, char* path);
    void init_master_playlists();
};

#endif //MY_HOST_LINUX_CONTEXT_H
