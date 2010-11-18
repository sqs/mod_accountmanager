#include "mod_accountmanager.h"

#include "http_protocol.h"
#include "http_log.h"
#include "apr_strings.h"

const char X_AMS_HEADER_KEY[] = "X-Account-Management-Status";
const char X_AMS_FORMAT[] = "active; authmethod=\"http-pake\"; id=\"%s\"";
const char X_AMS_INACTIVE[] = "none";
const char X_AMS_USER_VALID_CHARS[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@,.-_+ ";
const unsigned X_AMS_USER_MAX_LEN = 48;

const char AM_LINK_HEADER_KEY[] = "Link";
const char AM_LINK_FORMAT[] = "<%s>; rel=\"acct-mgmt\"";

/* The sample content handler */
static int add_am_status_header(request_rec *r)
{
    size_t user_len;
    char *am_status;

    if (!ap_is_initial_req(r))
        return DECLINED;

    if (!r->user || r->user[0] == '\0') {
        apr_table_mergen(r->headers_out, X_AMS_HEADER_KEY, X_AMS_INACTIVE);
        return DECLINED;
    }
    
    /* check shorter than X_AMS_USER_MAX_LEN */
    user_len = strnlen(r->user, X_AMS_USER_MAX_LEN+1);
    if (user_len > X_AMS_USER_MAX_LEN) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, 
                      "omitting %s header: " \
                      "username exceeds max length (%u)",
                      X_AMS_HEADER_KEY, X_AMS_USER_MAX_LEN);
        return DECLINED;
    }
        
    /* check for invalid chars */
    if (strspn(r->user, X_AMS_USER_VALID_CHARS) != user_len) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "omitting %s header: " \
                      "username includes invalid chars not in '%s'", 
                      X_AMS_HEADER_KEY, X_AMS_USER_VALID_CHARS);
        return DECLINED;
    }

    /* don't overwrite X-AMS header if app set it */
    if (apr_table_get(r->headers_out, X_AMS_HEADER_KEY))
        return DECLINED;

    am_status = apr_pcalloc(r->pool, strlen(X_AMS_FORMAT) + X_AMS_USER_MAX_LEN);
    if (!am_status) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "omitting %s header: couldn't pcalloc", X_AMS_HEADER_KEY);
        return DECLINED;
    }

    sprintf(am_status, X_AMS_FORMAT, r->user);
    apr_table_merge(r->headers_out, X_AMS_HEADER_KEY, am_status);
    return DECLINED;
}

static int add_am_link_header(request_rec *r) {
    char *am_link;
    accountmanager_config_rec *conf;

    conf = (accountmanager_config_rec *) ap_get_module_config(r->per_dir_config,
                                                              &accountmanager_module);

    /* not enabled for this dir */
    if (conf == NULL)
        return DECLINED;

    /* no AMCD URL set - browser can try to auto-discover host-meta file */
    if (!conf->amcd_url)
        return DECLINED;

    am_link = apr_pcalloc(r->pool, strlen(AM_LINK_FORMAT) + strlen(conf->amcd_url));
    if (!am_link) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "omitting Link header: couldn't pcalloc");
        return DECLINED;
    }

    sprintf(am_link, AM_LINK_FORMAT, conf->amcd_url);
    apr_table_merge(r->headers_out, AM_LINK_HEADER_KEY, am_link);
    return DECLINED;
}

static void accountmanager_register_hooks(apr_pool_t *p)
{
    ap_hook_access_checker(add_am_link_header, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_fixups(add_am_status_header, NULL, NULL, APR_HOOK_LAST);
}


/*************** CONFIG */

void *create_accountmanager_dir_config(apr_pool_t *p, char *dir) {
    accountmanager_config_rec *conf;

    if (dir == NULL)
        return NULL;

    conf = (accountmanager_config_rec *)apr_pcalloc(p, sizeof(accountmanager_config_rec));
    if (conf) {
        conf->dir_name = apr_pstrdup(p, dir);
    }
    
    return conf;
}

void *create_accountmanager_server_config(apr_pool_t *p, server_rec *server) {
    accountmanager_config_rec *conf;

    if (server == NULL)
        return NULL;

    conf = (accountmanager_config_rec *)apr_pcalloc(p, sizeof(accountmanager_config_rec));
    if (conf) {
        conf->server_hostname = apr_pstrdup(p, server->server_hostname);
    }
    
    return conf;
}


/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA accountmanager_module = {
    STANDARD20_MODULE_STUFF, 
    create_accountmanager_dir_config, /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL, /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    accountmanager_cmds,   /* table of config file commands       */
    accountmanager_register_hooks  /* register hooks                      */
};
