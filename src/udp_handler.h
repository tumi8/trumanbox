#ifndef _UDP_HANDLER_H_
#define _UDP_HANDLER_H_

struct udp_handler_t;

struct udp_handler_t* udphandler_create(int udpfd);
void udphandler_destroy(struct udp_handler_t* u);

void udphandler_run(struct udp_handler_t* u);

#endif
