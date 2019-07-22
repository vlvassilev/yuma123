#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <asm/errno.h>
#include <getopt.h>

#include "libtraffic-generator.h"
#include "timespec_math.h"

typedef struct raw_socket_t_ {
    int socket;
    unsigned int ifindex;
} raw_socket_t;

int raw_socket_init(char* if_name, raw_socket_t* raw_socket)
{
    int rc;
    int sock;
    struct ifreq ifr;
    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if ( socket < 0 ) {
        return sock;
    }
    raw_socket->socket=sock;

    strcpy(ifr.ifr_name, if_name);
    rc = ioctl(sock, SIOCGIFINDEX, &ifr);
    if ( rc < 0 ) {
	return rc;
    }
    raw_socket->ifindex=ifr.ifr_ifindex;

    return 0;
}

static int raw_socket_send(raw_socket_t* raw_socket, uint8_t* raw_frame, uint32_t raw_frame_len)
{
    struct sockaddr_ll ta;

    ta.sll_family = PF_PACKET; /* RAW communications */
    ta.sll_protocol = 0; /* Not used for sending */
    ta.sll_ifindex  = raw_socket->ifindex; /* Interface index */
    ta.sll_hatype = 0; /* Not used for sending */
    ta.sll_pkttype = 0; /* Not used for sending */
    ta.sll_halen = 6; /* All MAC addresses are 6 octets in size */
    memset(ta.sll_addr, 0, sizeof(ta.sll_addr));
    memcpy(ta.sll_addr, &raw_frame[0]/*destination mac*/, 6);

    int rc = sendto(raw_socket->socket, raw_frame, raw_frame_len, 0,(struct sockaddr*)&ta, sizeof(ta));
    if ( rc < 0 ) {
        return rc;
    }
    return 0;
}

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
    uint32_t interburst_gap=20;
    uint32_t frames_per_burst=0;
    uint32_t bursts_per_stream=0;
    uint64_t total_frames=0;

    int optc;
    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;

    while ((optc = getopt_long (argc, argv, "i:s:d:f:b:n:p:t", long_options, NULL)) != -1) {
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
            default:
                exit (-1);
        }
    }

    ret = raw_socket_init(interface_name /*e.g eth0*/, &raw_socket);
    assert(ret==0);

    tg = traffic_generator_init(frame_size, frame_data_hexstr, interframe_gap, interburst_gap, frames_per_burst, bursts_per_stream, total_frames);
    clock_gettime( CLOCK_MONOTONIC, &epoch);

    uint64_t frm=0;
    uint64_t print_sec=0;
    while(1) {
        uint8_t* cur_frame_data;
        uint32_t cur_frame_size;

        ret = traffic_generator_get_frame(tg, &cur_frame_size, &cur_frame_data, &tx_time_sec, &tx_time_nsec);
        if(ret!=0) {
            break;
        }
        clock_gettime( CLOCK_MONOTONIC, &now);
        rel.tv_sec = tx_time_sec;        /* seconds */
        rel.tv_nsec = tx_time_nsec;      /* nanoseconds */
        timespec_add(&rel, &epoch, &abs);
        timespec_sub(&now, &abs, &req);

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
