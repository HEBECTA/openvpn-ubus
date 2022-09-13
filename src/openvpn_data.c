#include "openvpn_data.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>

int connect_openvpn(int *sockfd){

        *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));

        addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, CLIENT_SOCK_FILE);

        if ( connect(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){

                syslog(LOG_ERR, "OPENVPN UBUS: Failed to connect to opnevpn server");

                return -1;
        }


        return 0;
}

int read_connected_clients(int sockfd, char *buff, int buff_len){

        //char *command = "status 3\n";
        char *command = "load-stats\n";

        send(sockfd, command, strlen(command), 0);

        recv(sockfd, buff, buff_len, 0);

        return 0;
}

int disconnect_client(const char *name){



        return 0;
}