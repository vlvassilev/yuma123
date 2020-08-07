#include <stdint.h>
typedef struct raw_socket_t_ {
    int socket;
    unsigned int ifindex;
} raw_socket_t;

int raw_socket_init(char* if_name, raw_socket_t* raw_socket);
int raw_socket_send(raw_socket_t* raw_socket, uint8_t* raw_frame, uint32_t raw_frame_len);
int raw_socket_receive(raw_socket_t* raw_socket, uint8_t* raw_frame, uint32_t raw_frame_max_len, uint32_t* raw_frame_received_len);
