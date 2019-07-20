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

#include "libtraffic-generator.h"

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
    printf(".");
    return 0;
}

static void ts_add(struct timespec* a, struct timespec* b, struct timespec* sum)
{
}

static void ts_sub(struct timespec* after, struct timespec* before, struct timespec* diff)
{
}

int main(int args, char** argv)
{
    int ret;
    raw_socket_t raw_socket;
    uint64_t tx_time_sec;
    uint32_t tx_time_nsec;
    uint32_t max_frame_len;
    uint8_t* frame_buf;
    uint32_t frame_len;

    struct timespec epoch,rel,abs,now,req,rem;

    traffic_generator_t* tg;

    ret = raw_socket_init(/* "eth0" */ argv[1], &raw_socket);
    assert(ret==0);
    tg = traffic_generator_init(/*argv[2]*/"<traffic-generator><frame-size>64</frame-size><frame-data>...</frame-data></traffic-generator>", &max_frame_len);
    clock_gettime( CLOCK_MONOTONIC, &epoch);
    frame_buf = malloc(max_frame_len);
    assert(frame_buf);

    while(1) {
        ret = traffic_generator_get_frame(tg, &frame_len, frame_buf, &tx_time_sec, &tx_time_nsec);
        assert(ret==0);
        clock_gettime( CLOCK_MONOTONIC, &now);
        rel.tv_sec = tx_time_sec;        /* seconds */
        rel.tv_nsec = tx_time_nsec;      /* nanoseconds */
        ts_add(&rel,&epoch,&abs);
        ts_sub(&abs,&now,&req);
        ret=nanosleep(&req,&rem);
        assert(ret==0);
        ret = raw_socket_send(&raw_socket, frame_buf, frame_len);
        assert(ret==0);
    }
}
