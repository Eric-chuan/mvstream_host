//
// Created by dy on 6/29/20.
//

#ifndef MY_HOST_LINUX_COMMON_H
#define MY_HOST_LINUX_COMMON_H

#include <string.h>
#include <unistd.h>
#include <cstdint>
#include <stdint.h>
#include <stdlib.h>
#include <cstdio>
#include <stdio.h>
#include <sys/time.h>
#include <vector>
#include <atomic>
#include <thread>
#include <fstream>
#include <semaphore.h>
#include <curl/curl.h>
#define MAX_LOG_CNT 10000
#define MAX_FIFO_SIZE (4096 * 2160 * 3 / 2)
#define FIFO_LEN 5
#define FRAME_SIZE (3840 * 2160 * 3 / 2)
#define MAX_INT32_LEN   (sizeof("-2147483648") - 1)
#define MAX_SEG_SIZE (3840 * 2160 / 3)
#define MAINVIEW 25
#define ASSISVIEW 65

typedef struct HLS_Media_Segment {
    char url[50];
    int64_t offset;
    int64_t size;
    uint64_t duration_ms;
    struct HLS_Media_Segment *next;
    struct HLS_Media_Segment *prev;
    uint8_t *data;
    int data_len;
} HLSMediaSegment;

typedef struct HLS_Master_Playlist {
    char *path;
    char *source;
} HLSMasterPlaylist;

#endif //MY_HOST_LINUX_COMMON_H
