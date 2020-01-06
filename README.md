# Tracker SparQL Service

This service creates a a simple HTTP endpoint for the [GNOME Tracker](https://gnome.pages.gitlab.gnome.org/tracker/) sparql database.

## Configuration
The configurartion file (you can provide it with `-c` command line option) is a simple key-value store (`key=value`). Currently supported configuration options:
 * `address`: the IP to bind to. `default: 0.0.0.0`
 * `port`: the tcp port to bind to. `default: 80`
 * `endpoint`: the URI where the sparql queries are served. `default: /sparql`
 * `log_level`: the log level. as usual the possible values are: FATAL, ERROR, WARN, INFO, DEBUG, TRACE. `default: INFO`
