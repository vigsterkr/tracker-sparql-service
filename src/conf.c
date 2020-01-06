#include "conf.h"

#include <libconfig.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "log.h"

static void default_conf(SparqlServerConfig* conf) {
    conf->addr = "0.0.0.0";
    conf->port = 80;
    conf->endpoint = "/sparql";
    log_set_level(LOG_INFO);
}

SparqlServerConfig* read_config(const char *path) {
    SparqlServerConfig *conf = NULL;
    conf = (SparqlServerConfig *)malloc(sizeof(SparqlServerConfig));
    if (conf == NULL) {
        log_error("Could not allocate memory for SparqldConfig!");
        return NULL;
    }

    if (path == NULL) {
        default_conf(conf);
        return conf;
    }

    config_t cfg;
    config_init(&cfg);

    int rc = config_read_file(&cfg, path);
    if (!rc) {
        log_error("%s:%d - %s", config_error_file(&cfg),
               config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return NULL;
    }

    const char* string_opt;
    rc = config_lookup_string(&cfg, "address", &string_opt);
    if (!rc) {
        log_info("Coniguration option 'address' has not been specified, binding to 0.0.0.0");
    } else {
        conf->addr = string_opt;
    }

    int port;
    rc = config_lookup_int(&cfg, "port", &port);
    if (!rc) {
        log_info("Coniguration option 'port' has not been specified, binding to port 80");
    } else {
        conf->port = port;
    }

    rc = config_lookup_string(&cfg, "endpoint", &string_opt);
    if (!rc) {
        log_info("Sparql endpoint is available at '%s", conf->endpoint);
    } else {
        conf->endpoint = string_opt;
    }

    rc = config_lookup_string(&cfg, "log_level", &string_opt);
    if (rc) {
        if (!strcmp(string_opt, "TRACE"))
            log_set_level(LOGGER_TRACE);
        else if (!strcmp(string_opt, "DEBUG"))
            log_set_level(LOGGER_DEBUG);
        else if (!strcmp(string_opt, "INFO"))
            log_set_level(LOGGER_INFO);
        else if (!strcmp(string_opt, "WARN"))
            log_set_level(LOGGER_WARN);
        else if (!strcmp(string_opt, "ERROR"))
            log_set_level(LOGGER_ERROR);
        else if (!strcmp(string_opt, "FATAL"))
            log_set_level(LOGGER_FATAL);
    }

    config_destroy(&cfg);
    /*
    if (reload == 1)
    {
        syslog(LOG_INFO, "Reloaded configuration file %s of %s",
               path,
               app_name);
    }
    else
    {
        syslog(LOG_INFO, "Configuration of %s read from file %s",
               app_name,
               path);
    }
*/
    return conf;
}
