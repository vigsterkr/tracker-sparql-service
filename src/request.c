#include "request.h"
#include "log.h"
#include "query.h"

struct evhttp *http_request_init(const SparQLServer *server) {
    struct evhttp *http = NULL;

    http = evhttp_new(server->ev);
    if (!http) {
        log_error("evhttp_new failed");
        return NULL;
    }

    evhttp_set_allowed_methods(http, EVHTTP_REQ_GET);
    evhttp_set_cb(http, server->config->endpoint, tracker_query, server->tracker);
    int rv = evhttp_bind_socket(http, server->config->addr, server->config->port);
    if (rv < 0) {
        log_error("evhttp_bind_socket failed");
        evhttp_free(http);
        return NULL;
    }

    return http;
}