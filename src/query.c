#include "query.h"
#include <syslog.h>
#include <stdio.h>

#include <event2/buffer.h>
#include <event2/keyvalq_struct.h>

#include "log.h"

TrackerSparqlConnection* tracker_init() {
    GError *error = NULL;
    TrackerSparqlConnection* conn = tracker_sparql_connection_get(NULL, &error);
    if (!conn) {
        log_error("Couldn't obtain a direct connection to the Tracker store: %s",
               error ? error->message : "unknown error");
        g_clear_error(&error);
        return NULL;
    }
    return conn;
}

void tracker_query(struct evhttp_request *req, void *arg) {
    TrackerSparqlConnection *conn = (TrackerSparqlConnection*)arg;
    GError *error = NULL;
    TrackerSparqlCursor *cursor = NULL;
    const struct evhttp_uri *ev_uri = NULL;
    const char *url_encoded_query = NULL;
    char *decoded_query = NULL;
    const char *raw_sparql_query = NULL;
    struct evkeyvalq params;

    if (conn == NULL) {
        log_error("TrackerSparqlConnection is NULL!");
        evhttp_send_error(req, HTTP_INTERNAL, 0);
        return;
    }

    // get ev_uri
    if ((ev_uri = evhttp_request_get_evhttp_uri(req)) == NULL) {
        log_error("Could not get URI!");
        evhttp_send_error(req, HTTP_INTERNAL, 0);
        return;
    }

    // parse query
    url_encoded_query = evhttp_uri_get_query(ev_uri);
    if (url_encoded_query == NULL) {
        log_info("Request has no query!");
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    decoded_query = evhttp_decode_uri(url_encoded_query);
    if (evhttp_parse_query_str(decoded_query, &params)) {
        log_info("Request has bad query '%s'", decoded_query);
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        free(decoded_query);
        return;
    }
    free(decoded_query);

    // get q= value
    raw_sparql_query = evhttp_find_header(&params, "q");
    if (raw_sparql_query == NULL) {
        log_debug("Query is missing 'q' key!");
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        evhttp_clear_headers (&params);
        return;
    }

    gchar *query = tracker_sparql_escape_string(raw_sparql_query);
    cursor = tracker_sparql_connection_query(conn,
                                             query,
                                             NULL,
                                             &error);
    g_free(query);
    evhttp_clear_headers(&params);

    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        log_error("Could not allocate ev buffer!");
        evhttp_send_error(req, HTTP_INTERNAL, 0);
        return;
    }

    if (error) {
        evbuffer_add_printf(buf, "Couldn't query the Tracker Store: '%s'",
               error ? error->message : "unknown error");
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
        g_clear_error(&error);
        evbuffer_free(buf);
        return;
    }

    /* Check results... */
    if (!cursor) {
        evbuffer_add_printf(buf, "No results found");
        evhttp_send_reply(req, HTTP_OK, "OK", buf);
    } else {
        int chunk_size = 0;
        evhttp_send_reply_start(req, HTTP_OK, "OK");

        // Iterate, synchronously, the results...
        glong cur_len;
        while (tracker_sparql_cursor_next(cursor, NULL, &error)) {
            evbuffer_add_printf(buf, "%s\n", tracker_sparql_cursor_get_string(cursor, 0, &cur_len));
            chunk_size += cur_len;
            if (chunk_size > 1200) {
                evhttp_send_reply_chunk(req, buf);
                chunk_size = 0;
            }
        }
        evhttp_send_reply_chunk(req, buf);
        g_object_unref(cursor);
        evhttp_send_reply_end(req);
    }
    evbuffer_free(buf);
}
