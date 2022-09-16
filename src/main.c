#include <syslog.h>
#include <errno.h>

#include "openvpn_data.h"
#include "openvpn_ubus.h"
#include "openvpn_uci.h"

#define SERVER_NAME_SIZE 50

int main(int argc, char *argv[]){

        int rc = 0;

	char server_name[SERVER_NAME_SIZE];

	rc = uci_get_opvpn_srv_name(server_name, SERVER_NAME_SIZE);
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to get openvpn server name (uci)");
		return ENODATA;
	}

        struct ubus_context *ctx = NULL;
	rc = init_start_ubus(ctx, server_name);
	if ( rc )
		syslog(LOG_ERR, "Openvpn-ubus: Failed to initialize and start ubus service");

	close_ubus(ctx);

        return rc;
}