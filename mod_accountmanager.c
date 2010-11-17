/* 
**  mod_auth_status.c -- Apache sample auth_status module
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "http_log.h"

const char X_AMS_HEADER_KEY[] = "X-Account-Management-Status";
const char X_AMS_FORMAT[] = "active; id=\"%s\"";
const char X_AMS_USER_VALID_CHARS[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@,.-_+ ";
const unsigned X_AMS_USER_MAX_LEN = 48;

/* The sample content handler */
static int add_am_status_header(request_rec *r)
{
    size_t user_len;
    char *am_status;

    if (!r->user)
        return DECLINED;

    if (!ap_is_initial_req(r))
        return DECLINED;

    
    /* check shorter than X_AMS_USER_MAX_LEN */
    user_len = strnlen(r->user, X_AMS_USER_MAX_LEN+1);
    if (user_len > X_AMS_USER_MAX_LEN) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, 
                      "omitting auth status header: " \
                      "username exceeds max length (%u)",
                      X_AMS_USER_MAX_LEN);
        return DECLINED;
    }
        
    /* check for invalid chars */
    if (strspn(r->user, X_AMS_USER_VALID_CHARS) != user_len) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "omitting auth status header: " \
                      "username includes invalid chars not in '%s'", 
                      X_AMS_USER_VALID_CHARS);
        return DECLINED;
    }

    am_status = apr_pcalloc(r->pool, strlen(X_AMS_FORMAT) + X_AMS_USER_MAX_LEN);
    sprintf(am_status, X_AMS_FORMAT, r->user);

    /* don't overwrite X-AMS header if app set it */
    if (!apr_table_get(r->headers_out, X_AMS_HEADER_KEY)) {
        apr_table_merge(r->headers_out, X_AMS_HEADER_KEY, am_status);
    }
   
    return DECLINED;
}

static void accountmanager_register_hooks(apr_pool_t *p)
{
    ap_hook_fixups(add_am_status_header, NULL, NULL, APR_HOOK_MIDDLE);
}

/* Dispatch list for API hooks */
module AP_MODULE_DECLARE_DATA accountmanager_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    accountmanager_register_hooks  /* register hooks                      */
};

