#include <stdint.h>

typedef struct burst_t_ {
} burst_t;

typedef struct stream_t_ {
    uint32_t frame_size;
    uint8_t* frame_data;
    uint32_t interframe_gap;
    uint32_t frames_per_burst;
    uint32_t interburst_gap;
    unsigned int bursts_per_stream;
    unsigned int burst_index;
    uint32_t interstream_gap;
} stream_t;

typedef struct traffic_generator_t_ {
    uint64_t total_frames;
    uint64_t total_frame_index;
    uint64_t sec;
    uint32_t nsec;
    float ns_per_octet;
    stream_t* streams;
    unsigned int streams_num;
    unsigned int stream_index;
    unsigned int burst_index;
    unsigned int frame_index;
} traffic_generator_t;


traffic_generator_t* traffic_generator_init(uint32_t frame_size, char* frame_data_hexstr, uint32_t interframe_gap, uint32_t interburst_gap, uint32_t frames_per_burst, uint32_t bursts_per_frame, uint64_t total_frames);
int traffic_generator_get_frame(traffic_generator_t* tg, uint32_t* frame_length, uint8_t** frame, uint64_t* tx_time_sec, uint32_t* tx_time_nsec);
