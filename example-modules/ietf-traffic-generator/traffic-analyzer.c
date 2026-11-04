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

#include "b64.h"
#include "yang_date_and_time.h"

#include "libtraffic-analyzer.h"
#include "timespec-math.h"
#include "raw-socket.h"
static struct option const long_options[] =
{
    {"interface-name", required_argument, NULL, 'i'},
    {"verbose", no_argument, NULL, 'v'},
    {"capture", no_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}
};

traffic_analyzer_t* ta;
static pthread_mutex_t ta_mutex = PTHREAD_MUTEX_INITIALIZER;

void* monitor(void* arg)
{
    int ret;
    int i;
    char date_and_time_str[] = "2024-11-05T12:34:56.000000000Z    ";
    uint8_t base64_buf[MAX_CAPTURE_FRAME_LEN*2];
    unsigned int retlen;

    while(1) {
        ret = getc(stdin);
        if(ret==EOF) {
            exit(0);
        }
        pthread_mutex_lock(&ta_mutex);
        ret = fprintf(stdout,"<state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-traffic-analyzer\"><statistics><pkts>%llu</pkts><octets>%llu</octets></statistics><testframe-statistics><pkts>%llu</pkts><sequence-errors>%llu</sequence-errors><latency><samples>%llu</samples><min>%llu</min><max>%llu</max><latest>%llu</latest></latency></testframe-statistics>",
                ta->totalframes,
                ta->totaloctets,
                ta->testframes,
                ta->testframe.sequence_errors,
                (uint64_t)ta->testframe.latency.samples,
                (uint64_t)ta->testframe.latency.min.tv_nsec,
                (uint64_t)ta->testframe.latency.max.tv_nsec,
                (uint64_t)ta->testframe.latency.last.tv_nsec);

        if(ta->capture_enabled && ta->totalframes>0) {
            ret = fprintf(stdout,"<capture>");
            ret = fprintf(stdout,"<frames>");
            for(i=0;i<MAX_CAPTURE_FRAMES && (uint64_t)i<ta->totalframes;i++) {
                unsigned int slot = (ta->totalframes - 1 - i) % MAX_CAPTURE_FRAMES;
                uint32_t capture_len = ta->capture_frame_size[slot] < MAX_CAPTURE_FRAME_LEN ?
                    ta->capture_frame_size[slot] : MAX_CAPTURE_FRAME_LEN;

                ret = fprintf(stdout,"<frame>");
                ret = fprintf(stdout,"<sequence-number>%llu</sequence-number>", (unsigned long long)(ta->totalframes-i));
                ieee_1588_to_yang_date_and_time(ta->capture_timestamp[slot].tv_sec, ta->capture_timestamp[slot].tv_nsec, date_and_time_str);
                ret = fprintf(stdout,"<timestamp>%s</timestamp>", date_and_time_str);
                ret = fprintf(stdout,"<size>%u</size>", (unsigned int)ta->capture_frame_size[slot]);
                b64_encode(ta->capture_frame_data[slot], capture_len, base64_buf, sizeof(base64_buf), 0, &retlen);
                ret = fprintf(stdout,"<data>%s",base64_buf);
                ret = fprintf(stdout,"</data>");
                ret = fprintf(stdout,"</frame>");
            }
            ret = fprintf(stdout,"</frames>");
            ret = fprintf(stdout,"</capture>");
        }
        ret = fprintf(stdout,"</state>\n");

        pthread_mutex_unlock(&ta_mutex);
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

    ta = calloc(1, sizeof(traffic_analyzer_t));
    assert(ta!=NULL);

    while ((optc = getopt_long (argc, argv, "i:vc", long_options, NULL)) != -1) {
        switch (optc) {
            case 'i':
                interface_name=optarg;
                break;
            case 'c':
                ta->capture_enabled=1;
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

        pthread_mutex_lock(&ta_mutex);
        traffic_analyzer_put_frame(ta, raw_frame_data, raw_frame_len, now.tv_sec, now.tv_nsec);
        pthread_mutex_unlock(&ta_mutex);

        if(verbose) {
            fprintf(stderr,"#%llu, size %09u latency %09u nsec\n", ta->totalframes, raw_frame_len, ta->testframe.latency.last.tv_nsec);
        }

    }
}
