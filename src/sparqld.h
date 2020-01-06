#ifndef __SPARQLD_H__
#define __SPARQLD_H__

#include "conf.h"

#include <event2/event.h>
#include <tracker-sparql.h>

typedef struct _SparQLServer SparQLServer;
struct _SparQLServer {
    int delay;
    char *conf_file_name;
    char *pid_file_name;
    char *log_file_name;
    int pid_fd;
    char *app_name;
    FILE *log_stream;
    SparqlServerConfig *config;
    struct event_base *ev;
    TrackerSparqlConnection *tracker;
};

int sparqld_init(SparQLServer*);
void sparqld_destroy(SparQLServer*);

#endif /** __SPARQLD_H__ **/
