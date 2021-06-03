//
// Created by xj on 9/26/20.
//
#include "Pull_Stream.h"

#define re_time 40000

extern float get_time_diff_ms(timeval start, timeval end);
sem_t Stream_Puller::start_pull;
sem_t Stream_Puller::init_done;

void Stream_Puller::init(FIFO ** input, FIFO ** output, int input_cnt, int output_cnt, Context *ctx, int id)
{
    this->input_cnt = input_cnt;
    this->input = input;
    this->output_cnt = output_cnt;
    this->output = output;
    //this->packet_cnt = ctx->packet_cnt > 0 ? ctx->packet_cnt + this->buffer_frames : 0;
    this->packet_cnt = ctx->packet_cnt;// + this->buffer_frames;
    this->ctx = ctx;
    this->pul_id = id;
    // if(this->pul_id == 0){
    //     sem_init(&start_pull, 0, 0);
    //     sem_init(&init_done, 0, 0);
    // }
}

void Stream_Puller::loop(MediaThreadLockClient lc, size_t id)
{
    int cnt = 0;
    timeval init_time, last_time;
    int dt, delay_sum;
    last_time.tv_sec = 0;
    last_time.tv_usec = 0;
    int scnt = 0;
    bool init_flag = true;
    //sem_post(&Stream_Puller::init_done);
    //sem_wait(&Stream_Puller::start_pull);
    if(this->pul_id == 0) {
        gettimeofday(&tv_init, NULL);
    }
    uint8_t* seg_data = (uint8_t*)malloc(MAX_SEG_SIZE);
    size_t seg_size;
    int seg_index = 0;
    while (true) {
        timeval start, end;
        gettimeofday(&start, NULL);
        sprintf(this->input_path, "/ssd/stitch/HLS/segment%d%d.ts", pul_id, seg_index);
        FILE* seg_file;
        if ((seg_file = fopen(this->input_path, "rb")) == NULL) {
            printf("open file error in puller %s\n", input_path);
        }
        memset(seg_data, 0, MAX_SEG_SIZE);
        seg_size = fread(seg_data, 1, MAX_SEG_SIZE, seg_file);
        fclose(seg_file);
        if (seg_size > 0){
            if (scnt == 0) {
                //gettimeofday(&init_time, NULL);
            } else {
                gettimeofday(&last_time, NULL);
                float diff_ms = 40.0 * scnt - get_time_diff_ms(tv_init, last_time);
                if(diff_ms > 0) {
                    usleep((long) diff_ms * 1000);
                }
            }
            //gettimeofday(&this->tout[fcnt++],NULL);
            this->output[0]->put(seg_data, seg_size, scnt);
            lc.syncThread(scnt % 2);
            usleep(850000);
            // if (id == 0) {
            //     printf("pull segment %d\n", scnt);
            // }
            scnt++;
            gettimeofday(&end, NULL);
            //printf("time use in puller: %ld \n", (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
            if (this->packet_cnt > 0 && scnt >= this->packet_cnt)
                break;
        }
        seg_index = (seg_index + 1) % 8;
	}
}
