#include "openvpn_ubus.h"

#include "openvpn_data.h"

#include <syslog.h>

static const struct blobmsg_policy client_policy[] = {
	[CLIENT_NAME] = { .name = "client_name", .type = BLOBMSG_TYPE_STRING },
};

static const struct ubus_method openvpn_methods[] = {
	UBUS_METHOD_NOARG("get", clients_get),
	UBUS_METHOD("kill", client_kill, client_policy)
};

static struct ubus_object_type openvpn_object_type =
	UBUS_OBJECT_TYPE("openvpn2", openvpn_methods);

struct ubus_object openvpn_object = {
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
	
        char clients_data[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE];
        int clients_n = read_connected_clients(clients_data);

	for ( int i = 0; i < clients_n; ++i ){

		blob_buf_init(&temp, 0);

		blobmsg_add_string(&temp, "Real Address", clients_data[i][1]);
		blobmsg_add_string(&temp, "Virtual Address", clients_data[i][2]);
		blobmsg_add_string(&temp, "Virtual IPv6 Address", clients_data[i][3]);
		blobmsg_add_string(&temp, "Bytes Received", clients_data[i][4]);
		blobmsg_add_string(&temp, "Bytes Sent", clients_data[i][5]);
		blobmsg_add_string(&temp, "Connected Since", clients_data[i][6]);

		blobmsg_add_field(&b, BLOBMSG_TYPE_TABLE, clients_data[i][0], blob_data(temp.head), blob_len(temp.head));

		blob_buf_free(&temp);
	}

	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

static int client_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	/*
	 * This structure is used to store the arguments which are passed
	 * through ubus.
	 * __COUNTER_MAX in this scenario is equal to 1.
	 * So this structure will hold only one variable.
	 * */
	struct blob_attr *tb[__CLIENT_MAX];
	struct blob_buf b = {};
	
	blobmsg_parse(client_policy, __CLIENT_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[CLIENT_NAME])
		return UBUS_STATUS_INVALID_ARGUMENT;

	/*
	 * This is the place where the value is extracted and appended to our
	 * variable.
	 * COUNTER_VALUE in this scenario is equal to 0. 0 indicates the first
	 * array element.
	 * blogmsg_get_u32 parses the value which is appended to the variable.
	 * */
	//count += blobmsg_get_u32(tb[CLIENT_NAME]);

	syslog(LOG_NOTICE, "kill method \n");

	if ( kill_client(blobmsg_get_string(tb[CLIENT_NAME])) )
		syslog(LOG_ERR, "Openvpn-ubus: Failed to kill client %s", blobmsg_get_string(tb[CLIENT_NAME]));

	/*
	 * This part of the method returns a messaged through ubus.
	 * */
	blob_buf_init(&b, 0);

	//blobmsg_add_u32(&b, "count", count);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}