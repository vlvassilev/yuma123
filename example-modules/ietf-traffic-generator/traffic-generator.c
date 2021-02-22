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


int main(int argc, char** argv)
{
    int ret;
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

    int optc;
    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;
    int stdout_mode = 0;

    while ((optc = getopt_long (argc, argv, "i:s:d:f:b:n:p:t:T:e:S:m", long_options, NULL)) != -1) {
        switch (optc) {
            case 'i':
                interface_name=optarg;
                break;
            case 's':
                frame_size = atoi(optarg);
                break;
            case 'd':
                frame_data_hexstr = optarg; /*hexstr*/
                break;
            case 'f':
                interframe_gap = atoi(optarg);
                break;
            case 'b':
                interburst_gap = atoi(optarg);
                break;
            case 'n':
                frames_per_burst = atoi(optarg);
                break;
            case 'p':
                bursts_per_stream = atoi(optarg);
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
                exit (-1);
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

    tg = traffic_generator_init(interface_speed, realtime_epoch, frame_size, frame_data_hexstr, interframe_gap, interburst_gap, frames_per_burst, bursts_per_stream, total_frames, testframe_type);

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
