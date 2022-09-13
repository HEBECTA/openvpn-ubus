#include <stdio.h>

#include "openvpn_data.h"
#include "openvpn_ubus.h"


int main(int argc, char *argv[]){

        int rc = 0;
        int sockfd;

        rc = connect_openvpn(&sockfd);

        // uci server name ?

        struct ubus_context *ctx;

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);
	ubus_add_object(ctx, &counter_object);
	uloop_run();

	ubus_free(ctx);
	uloop_done();

        

        close(sockfd);

        //unlink(CLIENT_SOCK_FILE);


        return 0;
}