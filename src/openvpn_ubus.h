#ifndef OPENVPN_UBUS_H
#define OPENVPN_UBUS_H

#include <libubox/blobmsg_json.h>
#include <libubus.h>

#define MAX_CLIENTS 50
#define MAX_ATTRIBUTES 7
#define MAX_ATTRIBUTE_SIZE 20

static int clients_get(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

static int client_kill(struct ubus_context *ctx, struct ubus_object *obj,
		      struct ubus_request_data *req, const char *method,
		      struct blob_attr *msg);

/*
 *The enumaration array is used to specifie how much arguments will our 
 * methods accepted. Also to say trough which index which argument will 
 * be reacheble.
 * 
 *  */

enum {
	CLIENT_NAME,
	__CLIENT_MAX
};

/*
 * This policy structure is used to determine the type of the arguments
 * that can be passed to some kind of method. 
 * This structure will be used in another structure applying this policy
 * to our selected method.
 * */

static const struct blobmsg_policy openvpn_policy[];

/*
 * This structure is used to register available methods.
 * If a method accepts arguments, the method should have a policy.
 * */

static const struct ubus_method openvpn_methods[];

/*
 * This structure is used to define the type of our object with methods.
 * */
 
static struct ubus_object_type openvpn_object_type;

/*
 * This structure is used to register our program as an ubus object
 * with our methods and other neccessary data. 
 * */

struct ubus_object openvpn_object;


#endif