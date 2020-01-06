#ifndef __SPARQLD_REQUEST_H__
#define __SPARQLD_REQUEST_H__

#include "sparqld.h"

#include <event2/http.h>

struct evhttp *http_request_init(const SparQLServer *server);

#endif /** __SPARQLD_REQUEST_H__ **/
