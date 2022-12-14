#include "openvpn_ubus.h"

#include "openvpn_data.h"

#include <syslog.h>
#include <errno.h>

static const struct blobmsg_policy client_policy[] = {
	[CLIENT_NAME] = { .name = "client_name", .type = BLOBMSG_TYPE_STRING },
};

static const struct ubus_method openvpn_methods[] = {
	UBUS_METHOD_NOARG("get", clients_get),
	UBUS_METHOD("kill", client_kill, client_policy)
};

static struct ubus_object_type openvpn_object_type =
	UBUS_OBJECT_TYPE("openvpn", openvpn_methods);

static struct ubus_object openvpn_object = {
	.name = "openvpn",
	.type = &openvpn_object_type,
	.methods = openvpn_methods,
	.n_methods = ARRAY_SIZE(openvpn_methods),
};

static int clients_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_buf b = {};
	struct blob_buf temp = {};
	
	blob_buf_init(&b, 0);

	struct wrapper *wrp = container_of(obj, struct wrapper, ubus);
	if ( wrp == NULL ){

		syslog(LOG_NOTICE, "Openvpn-ubus: Failed to access clients data");
		return EFAULT;
	}

	// critical section	S T A R T
	pthread_mutex_lock(wrp->hash_table_mutex);

	struct hash_table_iterator it;

	init_hash_table_iterator(wrp->ht, &it);

	while ( hash_table_return_next(&it) ){

		blob_buf_init(&temp, 0);

		blobmsg_add_string(&temp, "Real Address", it.current_node->real_address);
		blobmsg_add_string(&temp, "Virtual Address", it.current_node->virtual_address_v4);
		blobmsg_add_string(&temp, "Virtual IPv6 Address", it.current_node->virtual_address_v6);
		blobmsg_add_u32(&temp, "Bytes Received", it.current_node->bytes_received);
		blobmsg_add_u32(&temp, "Bytes Sent", it.current_node->bytes_sent);
		blobmsg_add_string(&temp, "Connected Since", it.current_node->connected_since);

		blobmsg_add_field(&b, BLOBMSG_TYPE_TABLE, it.current_node->name, blob_data(temp.head), blob_len(temp.head));

		blob_buf_free(&temp);
	}

	pthread_mutex_unlock(wrp->hash_table_mutex);
	// critical section	E N D
	
EXIT_CLIENT_GET_FUN:

	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

static int client_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_attr *tb[__CLIENT_MAX];
	struct blob_buf b = {};

	blobmsg_parse(client_policy, __CLIENT_MAX, tb, blob_data(msg), blob_len(msg));

	struct wrapper *wrp = container_of(obj, struct wrapper, ubus);
	if ( wrp == NULL ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to access clients data");
		return EFAULT;
	}
	
	if (!tb[CLIENT_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	// critical section	S T A R T
	pthread_mutex_lock(wrp->hash_table_mutex);

	if ( kill_client(wrp->ht, blobmsg_get_string(tb[CLIENT_NAME])) )
		syslog(LOG_ERR, "Openvpn-ubus: Failed to kill client %s", blobmsg_get_string(tb[CLIENT_NAME]));

	pthread_mutex_unlock(wrp->hash_table_mutex);
	// critical section	E N D

	blob_buf_init(&b, 0);

	blobmsg_add_string(&b, "Killed client", blobmsg_get_string(tb[CLIENT_NAME]));

	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

int init_start_ubus(struct ubus_context *ctx, const char *server_name, struct hash_table *ht, pthread_mutex_t *hash_table_mutex){

	int server_name_len = strlen(server_name);

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx)
		return ECONNREFUSED;
	
	ubus_add_uloop(ctx);

	//	U B U S		S E R V I C E		N A M E

	char ubus_service_name[UBUS_SERVICE_NAME_SIZE];
	strcpy(ubus_service_name, "openvpn.");

	if ( server_name_len + strlen(ubus_service_name) < UBUS_SERVICE_NAME_SIZE ){

		strncat(ubus_service_name, server_name, strlen(ubus_service_name) + server_name_len);
		ubus_service_name[UBUS_SERVICE_NAME_SIZE-1] = '\0';
		openvpn_object.name = ubus_service_name;
	}

	// *********************************************************

	struct wrapper wrp;
	wrp.ubus = openvpn_object;
	wrp.ht = ht;
	wrp.hash_table_mutex = hash_table_mutex;

	ubus_add_object(ctx, &wrp.ubus);
	uloop_run();

	return 0;
}

void close_ubus(struct ubus_context *ctx){

	if ( ctx != NULL )
		ubus_free(ctx);

	uloop_done();
}