#ifndef __SPARQLD_CONF_H__
#define __SPARQLD_CONF_H__

typedef struct _SparqlServerConfig SparqlServerConfig;
struct _SparqlServerConfig {
    const char *addr;
    unsigned short port;
    const char *endpoint;
};

SparqlServerConfig *read_config(const char *path);

#endif /** __SPARQLD_CONF_H__ **/
