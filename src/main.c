#include <syslog.h>
#include <pthread.h>
#include <signal.h>

#include "openvpn_data.h"
#include "openvpn_ubus.h"
#include "openvpn_uci.h"
#include "hash_table.h"

#define SERVER_NAME_SIZE 50
#define HASH_TABLE_SIZE 100

int main(int argc, char *argv[]){

        int rc = 0;
	char server_name[SERVER_NAME_SIZE];

	pthread_t data_read_thread;
	struct ubus_context *ctx = NULL;
	struct hash_table ht;
	pthread_mutex_t hash_table_mutex = PTHREAD_MUTEX_INITIALIZER;

	struct wrapped_ht_mutex wrp;
	wrp.hash_table = &ht;
	wrp.hash_table_mutex = &hash_table_mutex;

	rc = init_hash_table(&ht, HASH_TABLE_SIZE);
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to initialize memory for storing clients data");
		return rc;
	}

	rc = uci_get_opvpn_srv_name(server_name, SERVER_NAME_SIZE);
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to get openvpn server name (uci)");
		goto EXIT_MAIN_ERROR;
	}

	rc = init_data_read_thread(&data_read_thread, &wrp);
	if ( rc ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to initialize thread for reading openvpn data");
		return rc;
	}

	rc = init_start_ubus(ctx, server_name, &ht, &hash_table_mutex);
	if ( rc )
		syslog(LOG_ERR, "Openvpn-ubus: Failed to initialize and start ubus service");

	pthread_kill(data_read_thread, SIGTERM);

EXIT_MAIN_ERROR:

	free_hash_table(ht);

	close_ubus(ctx);

        return rc;
}