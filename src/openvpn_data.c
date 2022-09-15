#include "openvpn_data.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int connect_openvpn(){

        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

        if ( sockfd < 0 )
                return EPERM;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));

        addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, CLIENT_SOCK_FILE);

        if ( connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
                return ECONNREFUSED;

        return 0;
}

int kill_client(const char *client_name){

        char buff[250];

        strcpy(buff, "kill ");
        strncat(buff, client_name, strlen(client_name));
        strncat(buff, "\n", sizeof(char));

        send(sockfd, buff, strlen(buff), 0);

        recv(sockfd, buff, 300, 0);

        syslog(LOG_NOTICE, "kill msg %s\n", buff);

        if ( strstr(buff, "SUCCESS") == NULL )
                return 1;

        syslog(LOG_NOTICE, "killed\n");

        return 0;
}

int read_connected_clients(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE]){

        const char *command = "status 2\n";

        int clients_n = 0;

        char readBuff[BUFF_SIZE];
        const char *delim = "\n";
        char *token, *it;

        send(sockfd, command, strlen(command), 0);

        it = recv_message_no_header(readBuff);
        if ( it == NULL )
                return -1;

        token = strtok(it, delim);

        while( token != NULL && strstr(token, "CLIENT_LIST") != NULL ) {

                if ( process_client(buff, token, clients_n) ){

                        syslog(LOG_ERR, "Openvpn-ubus: Failed to process client data from openvpn server");
                        return -1;
                }

                ++clients_n;
           
                token = strtok(NULL, delim);
        }

        return clients_n;
}

int clean_initial_server_response(){

        char buff[500];

        if ( recv(sockfd, buff, 500, 0) < 0 )
                return ESTRPIPE;

        return 0;
}

static char *recv_message_no_header(char *buff){

        const char *delim = "CLIENT_LIST";

        recv(sockfd, buff, 300, 0);

        recv(sockfd, buff, BUFF_SIZE, 0);

        buff = strstr(buff, delim);

        return buff;
}

static int process_client(char buff[MAX_CLIENTS][MAX_ATTRIBUTES][MAX_ATTRIBUTE_SIZE], char *client_token, int client_n){

        char delim = ',';

        char *it_start = strchr(client_token, delim);
        char *it_end;

        for ( int i = 0; it_start != NULL && i < 7; ++i ){

                it_start += sizeof(char);

                it_end = strchr(it_start, delim);

                strncpy(buff[client_n][i], it_start, ( it_end - it_start));
                buff[client_n][i][(it_end - it_start)] = '\0';

                it_start = strchr(it_start, delim);
        }

        return 0;
}


void disconnect_openvpn(){

        close(sockfd);
}
