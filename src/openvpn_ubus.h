#ifndef OPENVPN_UBUS_H
#define OPENVPN_UBUS_H

#include <libubox/blobmsg_json.h>
#include <libubus.h>

static int clients_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

static int client_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

/*
 * Global variable which will be to store value
 * which will be passed over ubus. * 
 * */
static int count;

/*
 *The enumaration array is used to specifie how much arguments will our 
 * methods accepted. Also to say trough which index which argument will 
 * be reacheble.
 * 
 *  */

enum {
	COUNTER_VALUE,
	__COUNTER_MAX
};

/*
 * This policy structure is used to determine the type of the arguments
 * that can be passed to some kind of method. 
 * This structure will be used in another structure applying this policy
 * to our selected method.
 * */

static const struct blobmsg_policy counter_policy[] = {
	[COUNTER_VALUE] = { .name = "server_name", .type = BLOBMSG_TYPE_STRING },
};

/*
 * This structure is used to register available methods.
 * If a method accepts arguments, the method should have a policy.
 * */

static const struct ubus_method counter_methods[] = {
	UBUS_METHOD_NOARG("get", counter_get),
	UBUS_METHOD("kill", counter_add, counter_policy)
};

/*
 * This structure is used to define the type of our object with methods.
 * */
 
static struct ubus_object_type counter_object_type =
	UBUS_OBJECT_TYPE("openvpn", counter_methods);

/*
 * This structure is used to register our program as an ubus object
 * with our methods and other neccessary data. 
 * */

static struct ubus_object counter_object = {
	.name = "counter",
	.type = &counter_object_type,
	.methods = counter_methods,
	.n_methods = ARRAY_SIZE(counter_methods),
};


/*
 * This method is used as a callback function 
 * to return the value of our variable count.
 * All the arguments ar neccessary.
 * Using blobmsg object, our variable is packed ant returned 
 * through ubus server.
 * */
static int clients_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg)
{
	struct blob_buf b = {};
	
	blob_buf_init(&b, 0);

	blobmsg_add_u32(&b, "count", count);



	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}


/*
 * This method is used to read the argument value which is passed over ubus
 * and append that value to our global variable.
 * All the arguments are neccessary.
 * */
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
	struct blob_attr *tb[__COUNTER_MAX];
	struct blob_buf b = {};
	
	blobmsg_parse(counter_policy, __COUNTER_MAX, tb, blob_data(msg), blob_len(msg));
	
	if (!tb[COUNTER_VALUE])
		return UBUS_STATUS_INVALID_ARGUMENT;

	/*
	 * This is the place where the value is extracted and appended to our
	 * variable.
	 * COUNTER_VALUE in this scenario is equal to 0. 0 indicates the first
	 * array element.
	 * blogmsg_get_u32 parses the value which is appended to the variable.
	 * */
	count += blobmsg_get_u32(tb[COUNTER_VALUE]);

	/*
	 * This part of the method returns a messaged through ubus.
	 * */
	blob_buf_init(&b, 0);

	blobmsg_add_u32(&b, "count", count);
	ubus_send_reply(ctx, req, b.head);
	blob_buf_free(&b);

	return 0;
}

#endif