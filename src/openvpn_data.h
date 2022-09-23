#ifndef OPENVPN_DATA_H
#define OPENVPN_DATA_H

#include <pthread.h>

#include "openvpn_ubus.h"

#include "hash_table.h"

#define CLIENT_SOCK_FILE "/tmp/run/openvpn_management.sock"
#define BUFF_SIZE 10000
#define READ_TIME_INTERVAL 15

#define MAX_CLIENTS 50
#define MAX_ATTRIBUTES 7
#define MAX_ATTRIBUTE_SIZE 20

struct wrapped_ht_mutex {

        struct hash_table *hash_table;
        pthread_mutex_t *hash_table_mutex;
};

static int connect_openvpn(int *sockfd);

int kill_client(struct hash_table *ht, const char *client_name);

int read_connected_clients(struct hash_table *ht);

static int clean_initial_server_response(int sockfd);

static char *recv_message_no_header(int sockfd, char *buff);

static int process_client(struct hash_table *ht, char *client_token);

static void disconnect_openvpn(int sockfd);

int init_data_read_thread(pthread_t *thread_id, struct wrapped_ht_mutex *wrp);

static struct hash_table ht;

static int thread_run;

#endif