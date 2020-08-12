#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <asm/errno.h>
#include <getopt.h>
#include <pthread.h>

#include "libtraffic-analyzer.h"
#include "timespec-math.h"
#include "raw-socket.h"
static struct option const long_options[] =
{
    {"interface-name", required_argument, NULL, 'i'},
    {"verbose", no_argument, NULL, 'v'},
    {NULL, 0, NULL, 0}
};

traffic_analyzer_t* ta;

void* monitor(void* arg)
{
    int ret;
    while(1) {
        ret = getc(stdin);
        if(ret==EOF) {
            exit(0);
        }
        ret = fprintf(stdout,"<state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-traffic-analyzer\"><pkts>%llu</pkts><testframe-stats><testframe-pkts>%llu</testframe-pkts><latency><min>%llu</min><max>%llu</max><latest>%llu</latest></latency></testframe-stats></state>\n",
                ta->totalframes,
                ta->testframes,
                (uint64_t)ta->min_latency.tv_nsec,
                (uint64_t)ta->max_latency.tv_nsec,
                (uint64_t)ta->last_latency.tv_nsec);
        fflush(stdout);
        assert(ret>0);
        //sleep(1);
    }
}

int main(int argc, char** argv)
{
    int ret;
    raw_socket_t raw_socket;
    uint64_t tx_time_sec;
    uint32_t tx_time_nsec;
    uint8_t* raw_frame_data;
    uint32_t raw_frame_max_len;
    uint32_t raw_frame_len;
    char* interface_name;
    int verbose=0;
    uint32_t frame_size=64;

    int optc;
    struct timespec now;
    pthread_t monitor_tid;

    ta = malloc(sizeof(traffic_analyzer_t));
    memset(ta,sizeof(traffic_analyzer_t),0);

    while ((optc = getopt_long (argc, argv, "i:v", long_options, NULL)) != -1) {
        switch (optc) {
            case 'i':
                interface_name=optarg;
                break;
            case 'v':
                verbose=1;
                break;
            default:
                exit (-1);
        }
    }

    raw_frame_data = malloc(64*1024);
    assert(raw_frame_data!=NULL);
    raw_frame_max_len = 64*1024;

    ret = raw_socket_init(interface_name /*e.g eth0*/, &raw_socket);
    assert(ret==0);

    ret = pthread_create (&monitor_tid, NULL, monitor, NULL/*arg*/);
    assert(ret==0);

    while(1) {
        uint8_t* cur_frame_data;
        uint32_t cur_frame_size;

        ret = raw_socket_receive(&raw_socket, raw_frame_data, raw_frame_max_len, &raw_frame_len);
        assert(ret==0);
        clock_gettime( CLOCK_REALTIME, &now);

        traffic_analyzer_put_frame(ta, raw_frame_data, raw_frame_len, now.tv_sec, now.tv_nsec);

        if(verbose) {
            fprintf(stderr,"#%llu, size %09u latency %09u nsec\n", ta->totalframes, raw_frame_len, ta->last_latency.tv_nsec);
        }

    }
}
