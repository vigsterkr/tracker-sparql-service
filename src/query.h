#ifndef __SPARQLD_QUERY_H__
#define __SPARQLD_QUERY_H__

#include <tracker-sparql.h>
#include <event2/http.h>

TrackerSparqlConnection* tracker_init();
void tracker_query(struct evhttp_request *req, void *arg);

#endif /** __SPARQLD_QUERY_H__ **/
