#include "Pull_Stream.h"
#include "Context.h"
#include "HLSUpload.h"
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <vector>

using namespace std;

extern float get_time_diff_ms(timeval start, timeval end)
{
    long time_ms_end =  (end.tv_sec * 1000000 + end.tv_usec);
    long time_ms_start =  (start.tv_sec * 1000000 + start.tv_usec);
    return float(time_ms_end - time_ms_start) / 1000;
}

// extern std::atomic<int> sys_s_cnt;
// std::atomic<int> sys_s_cnt;

int main(int argc, char *argv[])
{
    char master_path[100] = "/usr/local/nginx/www/LiveHLS/master.m3u8";
    Context *host_ctx = new Context();
    int segment_num = 1000;
    host_ctx->init(segment_num, master_path);

    int fifo_len = 15;
    int fifo_num = MAINVIEW;
    long long fifo_data_size = fifo_len * MAX_SEG_SIZE;
    uint8_t * fifo_data = new uint8_t[fifo_data_size * fifo_num];

    FIFO **segment_puller_output = new FIFO * [MAINVIEW];
    FIFO **hls_uploader_input = new FIFO * [MAINVIEW];

    Stream_Puller **segment_puller = new Stream_Puller * [MAINVIEW];
    HLSUpload **hls_uploader = new HLSUpload * [MAINVIEW];

    int module_num = 2 * MAINVIEW;
    uint8_t * module_data = new uint8_t[module_num * MAX_SEG_SIZE];

    for(int i = 0; i < MAINVIEW; i++){
        segment_puller_output[i] = new FIFO();
        segment_puller_output[i]->init(fifo_len, MAX_SEG_SIZE, &fifo_data[i * fifo_data_size]);
        hls_uploader_input[i] = segment_puller_output[i];
    }
    for(int i = 0; i < MAINVIEW; i++){
        segment_puller[i] = new Stream_Puller();
        int sp_buf_index = 2 * i * MAX_SEG_SIZE;
        segment_puller[i]->set_buf(&module_data[sp_buf_index]);
        hls_uploader[i] = new HLSUpload();
        int hu_buf_index = sp_buf_index + MAX_SEG_SIZE;
        hls_uploader[i]->set_buf(&module_data[hu_buf_index]);

        segment_puller[i]->init(NULL, &segment_puller_output[i], 1, 1, host_ctx, i);
        hls_uploader[i]->init(&hls_uploader_input[i], NULL, 1, 1, host_ctx, i);
    }

    thread *segment_puller_thread = new thread[MAINVIEW];
    thread *hls_uploader_thread = new thread[MAINVIEW];

    MediaThreadLock                    sp_lock, hu_lock;
    std::vector<MediaThreadLockClient> sp_lockClients, hu_lockClients;
    sp_lockClients.resize(MAINVIEW);
    hu_lockClients.resize(MAINVIEW);
    for (size_t i = 0; i < MAINVIEW; i++) {
        sp_lock.clientReg(sp_lockClients[i]);
        hu_lock.clientReg(hu_lockClients[i]);
    }
    sp_lock.startWatcher();
    hu_lock.startWatcher();
    for (int i = 0; i < MAINVIEW; i++){
        segment_puller_thread[i] = std::thread(&Stream_Puller::loop, std::ref(segment_puller[i]), sp_lockClients[i], i);
        hls_uploader_thread[i] = std::thread(&HLSUpload::loop, std::ref(hls_uploader[i]), hu_lockClients[i], i);
    }
    for (int i = 0; i < MAINVIEW; i++){
        segment_puller_thread[i].detach();
        hls_uploader_thread[i].detach();
    }

    if(hls_uploader[0]->scnt_in >= segment_num) {
        sp_lock.stopWatcher();
        hu_lock.stopWatcher();
    }

    fprintf(stderr, "Waiting for stream.\n");

    while (!hls_uploader[0]->stop) {
        usleep(5000);
    }

    for(int i = 0; i < MAINVIEW; i++){
        segment_puller_output[i]->destroy();
    }
    for(int i = 0; i < MAINVIEW; i++){
        delete segment_puller[i];
        delete hls_uploader[i];
    }
    delete [] segment_puller;
    delete [] hls_uploader;

    delete [] segment_puller_output;
    delete [] hls_uploader_input;

    delete [] fifo_data;
    delete [] module_data;
    delete [] segment_puller_thread;
    delete [] hls_uploader_thread;
    return 0;
}