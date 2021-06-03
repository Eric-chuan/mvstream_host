//
// Created by xj on 12/10/20.
//

#include "Context.h"

void Context::init(int packet_cnt, char* path)
{
    this->num_of_streams = 25;
    this->packet_cnt = packet_cnt;
    this->master_path = path;
    init_master_playlists();
}

void Context::init_master_playlists()
{
    size_t len = sizeof("#EXTM3U\n"
                        "#EXT-X-VERSION:6\n") - 1;
    for (int i = 0; i < num_of_streams; i++) {
        len += sizeof("#EXT-X-STREAM-INF:RESOLUTION=3840x2160,CODECS=\"hevc\"\n"
                        "stream.m3u8\n") + 1 * MAX_INT32_LEN;
    }
    char* master_m3u8_data = (char*)malloc(len);
    int offset = 0;
    offset += sprintf(master_m3u8_data + offset, "#EXTM3U\n"
                        "#EXT-X-VERSION:6\n");

    for (int i = 0; i < num_of_streams; i++) {
        offset += sprintf(master_m3u8_data + offset, "#EXT-X-STREAM-INF:RESOLUTION=3840x2160,CODECS=\"hevc\"\n"
                        "stream%d.m3u8\n", i);
    }
    sprintf(master_m3u8_data + offset, "#EXT-X-ENDLIST\n\0");
    //printf("m3u8:%s\n", master_m3u8_data);
    FILE* master_m3u8_file = fopen(master_path, "wb");
    fwrite(master_m3u8_data, sizeof(char), strlen(master_m3u8_data) , master_m3u8_file);
    fclose(master_m3u8_file);
}
