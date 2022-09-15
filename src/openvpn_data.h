#ifndef OPENVPN_DATA_H
#define OPENVPN_DATA_H

#include "openvpn_ubus.h"

#define CLIENT_SOCK_FILE "/tmp/run/openvpn_management.sock"
#define BUFF_SIZE 5000

int connect_openvpn();

int kill_client(const char *client_name);

int read_connected_clients(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE]);

int clean_initial_server_response();

static char *recv_message_no_header(char *buff);

static int process_client(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE], char *client_token, int client_n);

void disconnect_openvpn();

static int sockfd;

#endif