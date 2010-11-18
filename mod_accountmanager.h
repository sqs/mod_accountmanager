#ifndef MOD_ACCOUNTMANAGER_H
#define MOD_ACCOUNTMANAGER_H

#include "httpd.h"
#include "ap_config.h"
#include "http_config.h"

module AP_MODULE_DECLARE_DATA accountmanager_module;

typedef struct accountmanager_config_struct {
    const char *server_hostname;
    const char *dir_name;
    const char *amcd_url;
} accountmanager_config_rec;

static const command_rec accountmanager_cmds[] = {
    AP_INIT_TAKE1("AccountManagerAMCDURL", 
                  ap_set_string_slot, 
                  (void *)APR_OFFSETOF(accountmanager_config_rec, amcd_url),
                  OR_AUTHCFG, "URL of AMCD file"),
    {NULL}
};


#endif // MOD_ACCOUNTMANAGER_H
