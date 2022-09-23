#include "openvpn_data.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

void sig_handler(int signum){

  thread_run = 0;

  syslog(LOG_NOTICE, "Openvpn-ubus: Received a SIGINT signal. Terminating program\n");
}


static int connect_openvpn(int *sockfd){

        *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

        if ( sockfd < 0 )
                return EPERM;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));

        addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, CLIENT_SOCK_FILE);

        if ( connect(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
                return ECONNREFUSED;

        if ( clean_initial_server_response(*sockfd) ){

                syslog(LOG_ERR, "Openvpn-ubus: Failed to clean initial server response from the stream");
                return ESTRPIPE;
        }

        return 0;
}

int kill_client(struct hash_table *ht, const char *client_name){

        int rc = 0;

        char buff[250];

        int sockfd;
        if ( connect_openvpn(&sockfd) ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to connect to openvpn server");
		return ECONNREFUSED;
	}

        strcpy(buff, "kill ");
        strncat(buff, client_name, strlen(client_name));
        strncat(buff, "\n", sizeof(char));

        send(sockfd, buff, strlen(buff), 0);

        if ( recv(sockfd, buff, 300, 0) < 0 ){

                rc = ESTRPIPE;
                goto EXIT_KILL_CLIENT_FUN;
        }

        if ( strstr(buff, "SUCCESS") == NULL ){

                rc = ENOMSG;
                goto EXIT_KILL_CLIENT_FUN;
        }

        rc = delete_client(ht, client_name);
        if ( rc ){

                syslog(LOG_ERR, "Openvpn-ubus: Failed to delete client from data buffer");
                goto EXIT_KILL_CLIENT_FUN;
        }

        syslog(LOG_NOTICE, "Openvpn-ubus: Killed %s\n", client_name);

EXIT_KILL_CLIENT_FUN:

        disconnect_openvpn(sockfd);

        return 0;
}

int read_connected_clients(struct hash_table *ht){

        int rc = 0;

        const char *command = "status 2\n";

        char readBuff[BUFF_SIZE];
        const char *delim = "\n";
        char *token, *it;

        int sockfd;
        if ( connect_openvpn(&sockfd) ){

		syslog(LOG_ERR, "Openvpn-ubus: Failed to connect to openvpn server");
		return ECONNREFUSED;
	}

        send(sockfd, command, strlen(command), 0);

        it = recv_message_no_header(sockfd, readBuff);
        if ( it == NULL ) // NULL -> no clients
                goto EXIT_READ_CLIENTS_FUN;

        token = strtok(it, delim);

        while( token != NULL && strstr(token, "CLIENT_LIST") != NULL ){

                if ( process_client(ht, token) ){

                        syslog(LOG_ERR, "Openvpn-ubus: Failed to process client data from openvpn server");
                        rc = ENOSR;
                        goto EXIT_READ_CLIENTS_FUN;
                }
  
                token = strtok(NULL, delim);
        }

EXIT_READ_CLIENTS_FUN:

        disconnect_openvpn(sockfd);

        return rc;
}

static int clean_initial_server_response(int sockfd){

        char buff[500];

        if ( recv(sockfd, buff, 500, 0) < 0 )
                return ESTRPIPE;

        return 0;
}

static char *recv_message_no_header(int sockfd, char *buff){

        const char *delim = "CLIENT_LIST";

        if ( recv(sockfd, buff, 300, 0) < 0 )
                return NULL;

        if ( recv(sockfd, buff, BUFF_SIZE, 0) < 0 )
                return NULL;

        buff = strstr(buff, delim);

        return buff;
}

static int process_client(struct hash_table *ht, char *client_token){

        int rc = 0;

        char client_buff[MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE];
        char delim = ',';

        char *it_start = strchr(client_token, delim);
        char *it_end;

        int i = 0;
        for ( ; it_start != NULL && i < MAX_ATTRIBUTES; ++i ){

                it_start += sizeof(char);

                it_end = strchr(it_start, delim);

                if ( (it_end - it_start) < MAX_ATTRIBUTE_SIZE ) {

                        strncpy(client_buff[i], it_start, (it_end - it_start) );
                        client_buff[i][(it_end - it_start)] = '\0';
                }

                it_start = strchr(it_start, delim);
        }
        
        if ( i < MAX_ATTRIBUTES )
                return ENOSR;

        struct node *client = get_client(ht, client_buff[0]);
        if ( client == NULL ){

                rc = insert_client(ht, client_buff[0], client_buff[1], client_buff[2], client_buff[3], client_buff[4], client_buff[5], client_buff[6]);

                if ( !rc )
                        syslog(LOG_NOTICE, "Openvpn-ubus: Connected %s\n", client_buff[0]);
        }
                

        else 
                update_client(client, client_buff[1], client_buff[2], client_buff[3], client_buff[4], client_buff[5], client_buff[6]);

        return rc;
}

static void disconnect_openvpn(int sockfd){

        close(sockfd);
}

void *read_clients_in_time_interval(void *vargp){

        signal(SIGTERM, sig_handler);

        thread_run = 1;

        struct wrapped_ht_mutex *wrp = (struct wrapped_ht_mutex *) vargp;

        while ( thread_run ){

                // critical section	S T A R T
	        pthread_mutex_lock(wrp->hash_table_mutex);

                int rc = read_connected_clients(wrp->hash_table);

                pthread_mutex_unlock(wrp->hash_table_mutex);
	        // critical section	E N D

                if ( rc < 0 )
                        syslog(LOG_ERR, "Openvpn-ubus: Failed to get clients from openvpn server");

                sleep(READ_TIME_INTERVAL);
        }

        return NULL;
}

int init_data_read_thread(pthread_t *thread_id, struct wrapped_ht_mutex *wrp){

        int rc = 0;

        rc = pthread_create(thread_id, NULL, read_clients_in_time_interval, (void *)wrp);

        return rc;
}
