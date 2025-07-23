#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <asm/errno.h>
#include <getopt.h>

#include "libtraffic-generator.h"
#include "timespec-math.h"
#include "raw-socket.h"

static struct option const long_options[] =
{
    {"interface-name", required_argument, NULL, 'i'},
    {"frame-size", required_argument, NULL, 's'},
    {"frame-data", required_argument, NULL, 'd'},
    {"interframe-gap", required_argument, NULL, 'f'},
    {"interburst-gap", required_argument, NULL, 'b'},
    {"frames-per-burst", required_argument, NULL, 'n'},
    {"bursts-per-stream", required_argument, NULL, 'p'},
    {"total-frames", required_argument, NULL, 't'},
    {"testframe-type", required_argument, NULL, 'T'},
    {"realtime-epoch", required_argument, NULL, 'e'},
    {"interface-speed", required_argument, NULL, 'S'},
    {"stdout-mode", no_argument, NULL, 'm'},
    {NULL, 0, NULL, 0}
};

void print_frame(uint64_t frame_index, uint32_t frame_size, uint8_t* frame_data, uint64_t tx_time_sec, uint32_t tx_time_nsec)
{
    int i;
    printf("%9llu %015llu:%09u %4u ", frame_index, tx_time_sec, tx_time_nsec, frame_size);
    for(i=0;i<frame_size;i++) {
        printf("%02X",frame_data[i]);
    }
    printf("\n");
}

#define MAX_STREAMS 2
int getopt_multistream(stream_t* streams, char* arg)
{
    int ret;
    int stream_index;
    if(0==memcmp(arg, "--frame-size", strlen("--frame-size"))) {
    	unsigned int frame_size;
    	ret = sscanf(arg+strlen("--frame-size"),"%u=%u",&stream_index,&frame_size);
        streams[stream_index-1].frame_size=frame_size;
    } else if(0==memcmp(arg, "--interframe-gap", strlen("--interframe-gap"))) {
    	unsigned int interframe_gap;
    	ret = sscanf(arg+strlen("--interframe-gap"),"%u=%u",&stream_index,&interframe_gap);
        streams[stream_index-1].interframe_gap=interframe_gap;
    } else if(0==memcmp(arg, "--interstream-gap", strlen("--interstream-gap"))) {
    	unsigned int interstream_gap;
    	ret = sscanf(arg+strlen("--interstream-gap"),"%u=%u",&stream_index,&interstream_gap);
        streams[stream_index-1].interstream_gap=interstream_gap;
    }  else if(0==memcmp(arg, "--frame-data", strlen("--frame-data"))) {
    	uint8_t* frame_data;
    	char* frame_data_hexstr;
    	ret = sscanf(arg+strlen("--frame-data"),"%u=",&stream_index);
    	frame_data_hexstr=strchr(arg, '=')+1;
    	frame_data = malloc(streams[stream_index-1].frame_size);
        memset(frame_data,0,streams[stream_index-1].frame_size);
        hexstr2bin(frame_data_hexstr, frame_data);
        streams[stream_index-1].frame_data=frame_data;

    } else {
        return -1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    int ret;
    int i;
    raw_socket_t raw_socket;
    uint64_t tx_time_sec;
    uint32_t tx_time_nsec;

    char* interface_name;
    uint32_t frame_size=64;
    char* frame_data_hexstr="000102030405060708090A0B";
    uint32_t interframe_gap=20;
    uint32_t interburst_gap=0;
    uint32_t frames_per_burst=0;
    uint32_t bursts_per_stream=0;
    uint64_t total_frames=0;
    char* testframe_type=NULL;
    char* realtime_epoch=NULL;
    uint64_t interface_speed=1000000000; /* 1G */
    char* src_mac_address=NULL;
    char* dst_mac_address=NULL;
    char* src_ipv4_address=NULL;
    char* dst_ipv4_address=NULL;
    char* src_udp_port=NULL;
    char* dst_udp_port=NULL;

    stream_t streams[MAX_STREAMS];
    unsigned int streams_num=1;


    int optc;
    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;
    int stdout_mode = 0;

    memset(streams,0,MAX_STREAMS*sizeof(stream_t));

    while ((optc = getopt_long (argc, argv, "i:s:d:f:b:n:p:t:T:e:S:m", long_options, NULL)) != -1) {
        switch (optc) {
            case 'i':
                interface_name=optarg;
                break;
            case 's':
                streams[0].frame_size = atoi(optarg);
                break;
            case 'd':
                streams[0].frame_data_hexstr = optarg; /*hexstr*/
                break;
            case 'f':
                streams[0].interframe_gap = atoi(optarg);
                break;
            case 'b':
                streams[0].interburst_gap = atoi(optarg);
                break;
            case 'n':
                streams[0].frames_per_burst = atoi(optarg);
                break;
            case 'p':
                streams[0].bursts_per_stream = atoi(optarg);
                break;
            case 't':
                total_frames = atoll(optarg);
                break;
            case 'T':
                testframe_type = optarg;
                break;
            case 'e':
                realtime_epoch = optarg;
                break;
            case 'S':
                interface_speed = atoll(optarg);
                break;
            case 'm':
                stdout_mode = 1;
                break;
            default:
                ret = getopt_multistream(streams, argv[optind-1]);
                if(ret!=0) {
                    printf("Invalid option optind=%d, optarg=%s, ret=%d\n", optind, optarg, ret);
                    return -1;
                }
        }
    }

    setenv("TZ", "UTC", 1);
    tzset();

    if(!stdout_mode) {
        ret = raw_socket_init(interface_name /*e.g eth0*/, &raw_socket);
        assert(ret==0);
    }

    if(realtime_epoch==NULL) {
        time_t sec;
        struct tm t;

        static char buf[] = "YYYY-MM-DDThh:mm:ss.nnnnnnnnnZ";

        clock_gettime( CLOCK_REALTIME/*CLOCK_MONOTONIC*/, &epoch);
        sec = epoch.tv_sec + 1; /* round up */
        assert (localtime_r(&sec, &t) != NULL);
        ret = strftime(buf, strlen(buf)+1, "%FT%T.000000000Z", &t);
        assert(ret==strlen("YYYY-MM-DDThh:mm:ss.nnnnnnnnnZ"));
        realtime_epoch = buf;
    }

    for(i=0;i<MAX_STREAMS;i++) {
        if(streams[i].frame_size!=0) {
            streams_num=i+1;
        }
    }

    tg = traffic_generator_init(interface_speed, realtime_epoch, total_frames, testframe_type, streams_num, streams);

    uint64_t frm=0;
    uint64_t print_sec=0;
    while(1) {
        uint8_t* cur_frame_data;
        uint32_t cur_frame_size;

        ret = traffic_generator_get_frame(tg, &cur_frame_size, &cur_frame_data, &tx_time_sec, &tx_time_nsec);
        if(ret!=0) {
            break;
        }
        if(stdout_mode) {
            print_frame(frm, cur_frame_size, cur_frame_data, tx_time_sec, tx_time_nsec);
        } else {
            clock_gettime( CLOCK_REALTIME /*CLOCK_MONOTONIC*/, &now);
            abs.tv_sec = tx_time_sec;        /* seconds */
            abs.tv_nsec = tx_time_nsec;      /* nanoseconds */
            timespec_sub(&abs, &now, &req);

            nanosleep(&req,&rem);

            ret = raw_socket_send(&raw_socket, cur_frame_data, cur_frame_size);
            assert(ret==0);
        }

        frm++;
    }
}
