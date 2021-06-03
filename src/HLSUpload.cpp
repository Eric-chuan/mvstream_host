#include "HLSUpload.h"

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct*)userp;
    if (mem->reserved == 0)
    {
        CURLcode res;
        double filesize = 0.0;

        res = curl_easy_getinfo(mem->c, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
        if((CURLE_OK == res) && (filesize>0.0))
        {
            mem->memory = (char*)realloc(mem->memory, (int)filesize + 2);
            if (mem->memory == NULL) {
                return 0;
            }
            mem->reserved = (int)filesize + 1;
        }
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

void SegQueue::init()
{
    this->capacity = 8;
    this->media_segments = new HLSMediaSegment * [8];
    for (int i = 0; i < 8; i++) {
        this->media_segments[i] = new HLSMediaSegment();
    }
    this->front = 0;
    this->tail = -1;
    this->count = 0;
}
void SegQueue::destroy()
{
    for (int i = 0; i < 8; i++) {
        free(this->media_segments[i]);
    }
    free(media_segments);
}

bool SegQueue::is_empty()
{
    return this->count == 0;
}

bool SegQueue::is_full()
{
    return this->count == this->capacity;
}

bool SegQueue::push(HLSMediaSegment *media_segment)
{
    if (is_full()) {
        printf("Queue is full\n");
        return false;
    }
    tail = (tail + 1) % this->capacity;
    memcpy(media_segments[tail], media_segment, sizeof(HLSMediaSegment));
    count++;
    return true;
}

bool SegQueue::pop(HLSMediaSegment *media_segment)
{
    if (is_empty()) {
        printf("Queue is empty\n");
        return false;
    }
    memcpy(media_segment, media_segments[front], sizeof(HLSMediaSegment));
    front = (front + 1) % this->capacity;
    count--;
    return true;
}

HLSMediaSegment* SegQueue::get_head()
{
    if (is_empty()) {
        printf("Queue is empty\n");
        return NULL;
    }
    return this->media_segments[front];
}

HLSMediaSegment* SegQueue::get_tail()
{
    if (is_empty()) {
        printf("Queue is empty\n");
        return NULL;
    }
    return this->media_segments[tail];
}

void HLSUpload::init(FIFO ** input, FIFO ** output, int input_cnt, int output_cnt, Context *ctx, int id)
{
    this->stream_id = id;
    this->packet_cnt = ctx->packet_cnt;
    this->input_cnt = input_cnt;
    this->input = input;
    this->output_cnt = output_cnt;
    this->output = output;
    this->scnt_in = 0;
    this->stop = false;
    this->media_playlists = (HLSMediaPlaylist*)malloc(sizeof(HLSMediaPlaylist));
    memset(this->media_playlists, 0x00, sizeof(HLSMediaPlaylist));
    this->media_playlists->segment_queue.init();

}
void HLSUpload::destroy()
{

}

void HLSUpload::loop(MediaThreadLockClient lc, size_t id)
{
    int scnt = 0;
    char seg_path[100];
    HLSMediaSegment *ms = new HLSMediaSegment();
    HLSMediaSegment *ms_pop = new HLSMediaSegment();
    ms_pop->data = (uint8_t*)malloc(MAX_SEG_SIZE);
    char ms_pop_path[100];
    while (true) {
        timeval start, end;
        gettimeofday(&start, NULL);
        this->input[0]->get(this->data_buf, this->seg_size, this->scnt_in);
        scnt++;
        sprintf(ms->url, "segment%d%d.ts", stream_id, (scnt - 1) % 8);
        if (this->media_playlists->segment_queue.is_full()) {
                this->media_playlists->segment_queue.pop(ms_pop);
                sprintf(ms_pop_path, "/usr/local/nginx/www/LiveHLS/%s", ms_pop->url);
                remove(ms_pop_path);
        }
        this->media_playlists->segment_queue.push(ms);
        sprintf(seg_path, "/usr/local/nginx/www/LiveHLS/segment%d%d.ts", stream_id, (scnt - 1) % 8);
        lc.syncThread((scnt) % 2);
        // uint8_t* tail_data = this->media_playlists->segment_queue.get_tail()->data;
        // int tail_data_len =  this->media_playlists->segment_queue.get_tail()->data_len;
        FILE* seg_file = fopen(seg_path, "wb");
        fwrite(this->data_buf, sizeof(uint8_t), this->seg_size, seg_file);
        fclose(seg_file);
        //lc.syncThread((scnt + 1) % 2);
        update_m3u8();
        gettimeofday(&end, NULL);
        int timeuse = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;
        usleep((900-timeuse)*1000);
        gettimeofday(&end, NULL);
        if (id == 0) {
            printf("upload segment %d\n", scnt_in);
            //printf("time use in uploader: %ld \n", (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
        }
        if (this->packet_cnt > 0 && scnt >= this->packet_cnt){
            usleep(30000);
            break;
        }
    }
    free(ms);
    free(ms_pop->data);
    free(ms_pop);
    this->stop = true;
}

void HLSUpload::update_m3u8()
{
    size_t len;
    int scnt, tail;
    char m3u8_path[100];
    len = sizeof("#EXTM3U\n"
                "#EXT-X-VERSION:6\n"
                "#EXT-X-TARGETDURATION:1\n"
                "#EXT-X-MEDIA-SEQUENCE:\n"
                "#EXT-X-INDEPENDENT-SEGMENTS\n") - 1
        + 1 * MAX_INT32_LEN;
    for (int j = 0; j < 8; j++) {
        len += sizeof("#EXTINF:1.000000,\n"
                            "segment.ts\n") - 1
                    + 1 * MAX_INT32_LEN;
    }
    char* m3u8_data = (char*)malloc(len);
    int offset = 0;
    offset += sprintf(m3u8_data + offset, "#EXTM3U\n"
                    "#EXT-X-VERSION:6\n"
                    "#EXT-X-TARGETDURATION:1\n"
                    "#EXT-X-MEDIA-SEQUENCE:0\n"
                    "#EXT-X-INDEPENDENT-SEGMENTS\n");
    scnt = this->media_playlists->segment_queue.count;
    tail = this->media_playlists->segment_queue.tail;
    for (int j = 0; j < scnt; j++) {
        offset += sprintf(m3u8_data + offset, "#EXTINF:1.000000,\n"
                            "%s\n", this->media_playlists->segment_queue.media_segments[(tail - j + 8) % 8]->url);
    }
    sprintf(m3u8_data + offset, "#EXT-X-ENDLIST\n\0");
    //printf("m3u8:%s\n", m3u8_data);
    sprintf(m3u8_path, "/usr/local/nginx/www/LiveHLS/stream%d.m3u8", stream_id);
    FILE* m3u8_file = fopen(m3u8_path, "wb");
    fwrite(m3u8_data, sizeof(char), strlen(m3u8_data) , m3u8_file);
    fclose(m3u8_file);
    free(m3u8_data);
}