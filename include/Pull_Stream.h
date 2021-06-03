//
// Created by xj on 9/26/20.
//

#ifndef MY_HOST_LINUX_PULL_STREAM_H
#define MY_HOST_LINUX_PULL_STREAM_H
#include "common.h"
#include "Module.h"
#include "MediaHostThreadLock.h"
#include "Context.h"
#include <atomic>



static timeval tv_init;

class Stream_Puller : public Module
{
private:
    char input_path[50];
    Context *ctx;
    uint8_t * data_buf;
    //timeval * tout;
public:
    static sem_t start_pull;
    static sem_t init_done;
    int pul_id;
    void init(FIFO ** input, FIFO ** output, int input_cnt, int output_cnt, Context *ctx, int id);
    void loop(MediaThreadLockClient lc, size_t id);
    int get_mem_cnt(){
        return 1;
    };
    void set_buf(uint8_t * buf) {
        this->data_buf = buf;
    };
};

#endif //MY_HOST_LINUX_PULL_STREAM_H

