#include "openvpn_uci.h"

#include <errno.h>

static struct uci_context* init_uci(const char *file, struct uci_package **pkg){

        struct uci_context *ctx = NULL;
        ctx = uci_alloc_context();
        if (ctx == NULL) {
		
                return NULL;
	}

        int rc = uci_set_confdir(ctx, CONFIG_PATH);
        if ( rc ){
                uci_free_context(ctx);
                return NULL;
        }

        rc = uci_load(ctx, file, pkg);
        if ( rc ){
                uci_free_context(ctx);
                return NULL;
        }

        return ctx;
}

int uci_get_opvpn_srv_name(char *server_name_buff, const int buff_len){

        int rc = 0;

        struct uci_context *uci_ctx = NULL;
        struct uci_package *package = NULL;

        uci_ctx = init_uci(OPENVPN_CONFIG_FILE, &package);
        if (uci_ctx == NULL)
                return ENODATA;
        
        struct uci_section *sct = uci_lookup_section(uci_ctx, package, OPENVPN_CONFIG_SECTION);

        if ( sct == NULL ){

                rc = ENODATA;
                goto EXIT_GET_SRV_NAME_FUN;
        }

        struct uci_option *o = uci_lookup_option(uci_ctx, sct, OPENVPN_SRV_NAME_OPTION);

        if ( o == NULL ){

                rc = ENODATA;
                goto EXIT_GET_SRV_NAME_FUN;
        }

        if ( o->type == UCI_TYPE_STRING ){

                int server_name_size = strlen(o->v.string);

                if ( !(server_name_size < buff_len) ){

                        rc = ENOBUFS;
                        goto EXIT_GET_SRV_NAME_FUN;
                }

                strncpy(server_name_buff, o->v.string, server_name_size);
                server_name_buff[server_name_size] = '\0';
        }       

        else
                rc = ENODATA;

EXIT_GET_SRV_NAME_FUN:

        uci_free_context(uci_ctx);

        return rc;
}