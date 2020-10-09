#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <asm/errno.h>
#include <getopt.h>

#include "libtraffic-generator.h"

static struct option const long_options[] =
{
    {"frame-size", required_argument, NULL, 's'},
    {"frame-data", required_argument, NULL, 'd'},
    {"src-mac-addresss", required_argument, NULL, 'a'},
    {"dst-mac-addresss", required_argument, NULL, 'A'},
    {"src-ipv4-address", required_argument, NULL, 'v'},
    {"dst-ipv4-address", required_argument, NULL, 'V'},
    {"src-ipv4-udp-port", required_argument, NULL, 'p'},
    {"dst-ipv4-udp-port", required_argument, NULL, 'P'},
    {"ipv4-ttl", required_argument, NULL, 't'},

    {NULL, 0, NULL, 0}
}; 

int main(int argc, char** argv)
{
    int ret;
    uint32_t frame_size=64;
    char* frame_data_hexstr=NULL;
    char* src_mac_address=NULL;
    char* dst_mac_address=NULL;
    char* src_ipv4_address=NULL;
    char* dst_ipv4_address=NULL;
    char* src_ipv4_udp_port=NULL;
    char* dst_ipv4_udp_port=NULL;
    char* ipv4_ttl=NULL;

    int optc;
    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;
    int stdout_mode = 0;

    while ((optc = getopt_long (argc, argv, "s:d:a:A:v:V:p:P:t:", long_options, NULL)) != -1) {
        switch (optc) {
            case 's':
                frame_size = atoi(optarg);
                break;
            case 'd':
                frame_data_hexstr = optarg; /*hexstr*/
                break;
            case 'm':
                stdout_mode = 1;
                break;
            case 'a':
                src_mac_address = optarg;
                break;
            case 'A':
                dst_mac_address = optarg;
                break;
            case 'v':
                src_ipv4_address = optarg;
                break;
            case 'V':
                dst_ipv4_address = optarg;
                break;
            case 'p':
                src_ipv4_udp_port = optarg;
                break;
            case 'P':
                dst_ipv4_udp_port = optarg;
                break;
            case 't':
                ipv4_ttl = optarg;
                break;
            default:
                exit (-1);
        }
    }

    frame_data_hexstr = traffic_generator_make_testframe(frame_size, frame_data_hexstr, src_mac_address, dst_mac_address, src_ipv4_address, dst_ipv4_address, ipv4_ttl, src_ipv4_udp_port, dst_ipv4_udp_port);

    printf("%s\n", frame_data_hexstr);

    return 0;
}
