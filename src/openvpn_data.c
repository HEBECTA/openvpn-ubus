#include "openvpn_data.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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

int kill_client(const char *client_name){

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

        syslog(LOG_NOTICE, "Openvpn-ubus: Killed %s\n", client_name);

EXIT_KILL_CLIENT_FUN:

        disconnect_openvpn(sockfd);

        return 0;
}

int read_connected_clients(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE]){

        const char *command = "status 2\n";

        int clients_n = 0;

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
        if ( it == NULL ){

                clients_n = -1;
                goto EXIT_READ_CLIENTS_FUN;
        }

        token = strtok(it, delim);

        while( token != NULL && strstr(token, "CLIENT_LIST") != NULL ) {

                if ( process_client(buff, token, clients_n) ){

                        syslog(LOG_ERR, "Openvpn-ubus: Failed to process client data from openvpn server");
                        clients_n = -1;
                        goto EXIT_READ_CLIENTS_FUN;
                }

                ++clients_n;
           
                token = strtok(NULL, delim);
        }

EXIT_READ_CLIENTS_FUN:

        disconnect_openvpn(sockfd);

        return clients_n;
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

static int process_client(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE], char *client_token, int client_n){

        char delim = ',';

        char *it_start = strchr(client_token, delim);
        char *it_end;

        for ( int i = 0; it_start != NULL && i < MAX_ATTRIBUTES; ++i ){

                it_start += sizeof(char);

                it_end = strchr(it_start, delim);

                if ( (it_end - it_start) < MAX_ATTRIBUTE_SIZE ) {

                        strncpy(buff[client_n][i], it_start, (it_end - it_start) );
                        buff[client_n][i][(it_end - it_start)] = '\0';
                }

                it_start = strchr(it_start, delim);
        }

        return 0;
}


static void disconnect_openvpn(int sockfd){

        close(sockfd);
}
