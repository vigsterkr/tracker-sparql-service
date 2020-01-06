#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "log.h"
#include "query.h"
#include "request.h"
#include "sparqld.h"

struct event_base *ev;
static int pid_fd = 0;
static char* pid_file_name;

int sparqld_init(SparQLServer *sparql_server) {
    sparql_server->delay = 1;
    sparql_server->conf_file_name = NULL;
    sparql_server->pid_file_name = NULL;
    sparql_server->log_file_name = NULL;
    sparql_server->pid_fd = -1;
    sparql_server->tracker = NULL;

    // create event base
    if ((sparql_server->ev = ev = event_base_new()) == NULL) {
        log_error("Error while event_base_new(): %s", strerror(errno));
        sparqld_destroy(sparql_server);
        return -1;
    }

    // tracker connection
    if ((sparql_server->tracker = tracker_init()) == NULL) {
        sparqld_destroy(sparql_server);
        return -1;
    }

    return 0;
}

void sparqld_destroy(SparQLServer *sparql_server) {
    /* Free allocated memory */
    if (sparql_server->conf_file_name != NULL)
        free(sparql_server->conf_file_name);
    if (sparql_server->pid_file_name != NULL)
        free(sparql_server->pid_file_name);
    if (sparql_server->log_file_name != NULL)
        free(sparql_server->log_file_name);
    if (sparql_server->config != NULL)
        free(sparql_server->config);
    if (sparql_server->ev != NULL)
        event_base_free(sparql_server->ev);
    if (sparql_server->tracker != NULL)
        g_object_unref(sparql_server->tracker);
}

/**
 * \brief Callback function for handling signals.
 * \param	sig	identifier of signal
 */
void handle_signal(int sig)
{
    if (sig == SIGINT)
    {

        log_debug("Stopping daemon ...");
        /* Unlock and close lockfile */
        if (pid_fd != -1)
        {
            lockf(pid_fd, F_ULOCK, 0);
            close(pid_fd);
        }
        /* Try to delete lockfile */
        if (pid_file_name != NULL)
        {
            unlink(pid_file_name);
        }
        event_base_loopbreak(ev);
        /* Reset signal handling to default behavior */
        signal(SIGINT, SIG_DFL);
    }
    else if (sig == SIGHUP)
    {
        log_debug("Reloading daemon config file ...");
        //read_conf_file(1);
    }
    else if (sig == SIGCHLD)
    {
        log_debug("Received SIGCHLD signal");
    }
}

/**
 * \brief This function will daemonize this app
 */
static void daemonize(SparQLServer* server)
{
    pid_t pid = 0;
    int fd;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Ignore signal sent from child to parent process */
    signal(SIGCHLD, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--)
        close(fd);

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    /* Try to write PID of daemon to lockfile */
    if (server->pid_file_name != NULL) {
        char str[256];
        server->pid_fd = open(server->pid_file_name, O_RDWR | O_CREAT, 0640);
        if (server->pid_fd < 0) {
            /* Can't open lockfile */
            exit(EXIT_FAILURE);
        }
        if (lockf(server->pid_fd, F_TLOCK, 0) < 0) {
            /* Can't lock file */
            exit(EXIT_FAILURE);
        }
        pid_fd = server->pid_fd;

        /* Get current PID */
        sprintf(str, "%d\n", getpid());
        /* Write PID to lockfile */
        write(server->pid_fd, str, strlen(str));
    }
}

/**
 * \brief Print help for this application
 */
void print_help(void)
{
    printf("\n Usage: tracker-sparqld [OPTIONS]\n\n");
    printf("  Options:\n");
    printf("   -h --help                 Print this help\n");
    printf("   -c --conf_file filename   Read configuration from the file\n");
	printf("   -l --log_file  filename   Write logs to the file\n");
    printf("   -d --daemon               Daemonize this application\n");
    printf("   -p --pid_file  filename   PID file used by daemonized app\n");
    printf("\n");
}

/* Main function */
int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"conf_file", required_argument, 0, 'c'},
		{"log_file", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"daemon", no_argument, 0, 'd'},
        {"pid_file", required_argument, 0, 'p'},
        {NULL, 0, 0, 0}};
    int value, option_index = 0, ret;
    int start_daemonized = 0;
    SparQLServer server;
    struct evhttp *http = NULL;

    sparqld_init(&server);

    /* Try to process all command line arguments */
    while ((value = getopt_long(argc, argv, "c:p:l:dh", long_options, &option_index)) != -1)
    {
        switch (value)
        {
        case 'c':
            server.conf_file_name = strdup(optarg);
            break;
        case 'p':
            server.pid_file_name = strdup(optarg);
            break;
        case 'l':
			server.log_file_name = strdup(optarg);
			break;
        case 'd':
            start_daemonized = 1;
            break;
        case 'h':
            print_help();
            return EXIT_SUCCESS;
        case '?':
            print_help();
            return EXIT_FAILURE;
        default:
            break;
        }
    }

    /* When daemonizing is requested at command line. */
    if (start_daemonized == 1)
        daemonize(&server);

    /* Open system log and write message to it */
    openlog(argv[0], LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started tracker-sparqld");

    /* Daemon will handle two signals */
    signal(SIGINT, handle_signal);
    signal(SIGHUP, handle_signal);

    /* Try to open log file to this daemon */
    log_init(server.log_file_name);

    /* Read configuration from config file */
    server.config = read_config(server.conf_file_name);

    // bind to the given address and port
    http = http_request_init(&server);
    if (!http) {
        sparqld_destroy(&server);
        return -1;
    }

    event_base_loop(server.ev, EVLOOP_NO_EXIT_ON_EMPTY);

    /* Close log file, when it is used. */
    log_close();

    /* Write system log and close it. */
    syslog(LOG_INFO, "Stopped tracker-sparqld");
    closelog();

    evhttp_free(http);
    sparqld_destroy(&server);

    return EXIT_SUCCESS;
}
