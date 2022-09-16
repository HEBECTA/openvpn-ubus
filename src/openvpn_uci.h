#ifndef OPENVPN_UCI_H
#define OPENVPN_UCI_H

#include <uci.h>

#define CONFIG_PATH "/etc/config/"
#define OPENVPN_CONFIG_FILE "openvpn"
#define OPENVPN_CONFIG_SECTION "serveris"
#define OPENVPN_SRV_NAME_OPTION "_name"

static struct uci_context* init_uci(const char *file, struct uci_package **pkg);

int uci_get_opvpn_srv_name(char *server_name_buff, const int buff_len);

#endif