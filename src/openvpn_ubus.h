#ifndef OPENVPN_UBUS_H
#define OPENVPN_UBUS_H

#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include "hash_table.h"

#define UBUS_SERVICE_NAME_SIZE 100

#define MAX_CLIENTS 50
#define MAX_ATTRIBUTES 7
#define MAX_ATTRIBUTE_SIZE 20

struct wrapper {

	struct ubus_object ubus;
	struct hash_table *ht;
	pthread_mutex_t *hash_table_mutex;
};

static int clients_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

static int client_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);


enum {
	CLIENT_NAME,
	__CLIENT_MAX
};

static const struct blobmsg_policy openvpn_policy[];

static const struct ubus_method openvpn_methods[];
 
static struct ubus_object_type openvpn_object_type;

static struct ubus_object openvpn_object;

int init_start_ubus(struct ubus_context *ctx, const char *server_name, struct hash_table *ht, pthread_mutex_t *hash_table_mutex);

void close_ubus(struct ubus_context *ctx);

#endif