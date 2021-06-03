#include "common.h"
#include "Module.h"
#include "Context.h"
#include <atomic>
#include "MediaHostThreadLock.h"
#include <fstream>

#define STRING 0x0001
#define BINARY 0x0002
#define MASTER_PLAYLIST 0
#define MEDIA_PLAYLIST 1

#define PROGRAM_STREAM_MAP  0xBC
#define PADDING_STREAM  0xBE
#define PRIVATE_STREAM_2  0xBF
#define ECM_STREAM  0xF0
#define EMM_STREAM 0xF1
#define PROGRAM_STREAM_DIRECTORY  0xFF
#define DSMCC_STREAM  0xF2
#define E_STREAM  0xF8



typedef struct Memory_Struct {
    char *memory;
    size_t size;
    size_t reserved;
    CURL *c;
} MemoryStruct;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

class SegQueue
{
public:
    HLSMediaSegment **media_segments;
    int capacity;
    int front;
    int tail;
    int count;
public:
    void init();
    void destroy();
    bool is_empty();
    bool is_full();
    bool push(HLSMediaSegment *media_segment);
    bool pop(HLSMediaSegment *media_segment);
    HLSMediaSegment* get_head();
    HLSMediaSegment* get_tail();
};

typedef struct HLS_Media_Playlist {
    char *path;
    char *source;
    uint64_t target_duration_ms;
    uint64_t total_duration_ms;
    bool is_endlist;
    int first_media_sequence;
    int last_media_sequence;
    SegQueue segment_queue;
    HLSMediaSegment *first_media_segment;
    HLSMediaSegment *last_media_segment;
    struct HLS_Media_Playlist *next;
} HLSMediaPlaylist;

class HLSUpload: public Module
{
public:
    int stream_id;
    HLSMediaPlaylist* media_playlists;
    long long  seg_size;
    uint8_t * data_buf;
    int scnt_in;
    std::atomic<bool> stop;
public:
    void init(FIFO ** input, FIFO ** output, int input_cnt, int output_cnt, Context *ctx, int id);
    void destroy();
    void loop(MediaThreadLockClient lc, size_t id);
    void update_m3u8();
    void set_buf(uint8_t * buf) {
        this->data_buf = buf;
    };
};
