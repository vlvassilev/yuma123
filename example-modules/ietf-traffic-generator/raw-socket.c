#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include "raw-socket.h"

int raw_socket_init(char* if_name, raw_socket_t* raw_socket)
{
    int rc;
    int sock;
    struct sockaddr_ll addr = {0};
    struct ifreq ifr = {0};

    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if ( socket < 0 ) {
        return sock;
    }
    raw_socket->socket=sock;

    strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1);
    rc = ioctl(sock, SIOCGIFINDEX, &ifr);
    if ( rc < 0 ) {
	return rc;
    }
    raw_socket->ifindex=ifr.ifr_ifindex;

    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifindex;
    addr.sll_protocol = htons(ETH_P_ALL);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        assert(0);
    }

    memset((void*)&ifr,sizeof(struct ifreq),0);
    /* promiscuous mode */
    rc = ioctl(sock, SIOCGIFFLAGS, &ifr);
    assert(rc==0);
    ifr.ifr_flags |= IFF_PROMISC;
    rc = ioctl(sock, SIOCSIFFLAGS, &ifr);
    assert(rc==0);

    return 0;
}

int raw_socket_send(raw_socket_t* raw_socket, uint8_t* raw_frame, uint32_t raw_frame_len)
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

int raw_socket_receive(raw_socket_t* raw_socket, uint8_t* raw_frame, uint32_t raw_frame_max_len, uint32_t* raw_frame_received_len)
{
    ssize_t res;
    struct sockaddr_ll saddr;
    int saddr_len = sizeof (saddr);

    do {
        res = recvfrom(raw_socket->socket, raw_frame, raw_frame_max_len, 0, (struct sockaddr *)&saddr, &saddr_len);
        if(res<=0) {
            perror("recvfrom");
            assert(0);
        }
    } while(saddr.sll_pkttype == PACKET_OUTGOING);

    *raw_frame_received_len = res;

    return 0;
}

