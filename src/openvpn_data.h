#ifndef OPENVPN_DATA_H
#define OPENVPN_DATA_H

#define CLIENT_SOCK_FILE "/tmp/run/openvpn_management.sock"

int connect_openvpn(int *sockfd);

int read_connected_clients(int sockfd, char *buff, int buff_len);

int disconnect_client(const char *name);


#endif