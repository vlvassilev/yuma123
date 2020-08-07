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
    {"testframe", required_argument, NULL, 'T'},
    {NULL, 0, NULL, 0}
};

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
    int testframe=0;

    int optc;
    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;

    while ((optc = getopt_long (argc, argv, "i:s:d:f:b:n:p:t:T", long_options, NULL)) != -1) {
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
                if(0==strcmp(optarg,"true")) {
                    testframe = 1;
                }
                break;
            default:
                exit (-1);
        }
    }

    ret = raw_socket_init(interface_name /*e.g eth0*/, &raw_socket);
    assert(ret==0);

    tg = traffic_generator_init(frame_size, frame_data_hexstr, interframe_gap, interburst_gap, frames_per_burst, bursts_per_stream, total_frames, testframe);
    clock_gettime( CLOCK_REALTIME/*CLOCK_MONOTONIC*/, &epoch);
    traffic_generator_set_epoch(tg, epoch.tv_sec, epoch.tv_nsec);

    uint64_t frm=0;
    uint64_t print_sec=0;
    while(1) {
        uint8_t* cur_frame_data;
        uint32_t cur_frame_size;

        ret = traffic_generator_get_frame(tg, &cur_frame_size, &cur_frame_data, &tx_time_sec, &tx_time_nsec);
        if(ret!=0) {
            break;
        }
        clock_gettime( CLOCK_REALTIME /*CLOCK_MONOTONIC*/, &now);
        abs.tv_sec = tx_time_sec;        /* seconds */
        abs.tv_nsec = tx_time_nsec;      /* nanoseconds */
        timespec_sub(&abs, &now, &req);

        nanosleep(&req,&rem);

        ret = raw_socket_send(&raw_socket, cur_frame_data, cur_frame_size);
        assert(ret==0);

        if(now.tv_sec>print_sec) {
            print_sec=now.tv_sec;
            printf("%llu\n",frm);
        }
        frm++;
    }
}
