#include <stdio.h>
#include <syslog.h>

#include "openvpn_data.h"
#include "openvpn_ubus.h"


int main(int argc, char *argv[]){

        int rc = 0;

        rc = connect_openvpn();
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to connect to openvpn server");
		return rc;
	}

	rc = clean_initial_server_response();
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Dump read error");
		goto EXIT_PROGRAM;
	}

        // uci server name ?

        struct ubus_context *ctx;

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);
	ubus_add_object(ctx, &openvpn_object);
	uloop_run();

	ubus_free(ctx);
	uloop_done();

        

        disconnect_openvpn();

        //unlink(CLIENT_SOCK_FILE);
EXIT_PROGRAM:

        return rc;
}