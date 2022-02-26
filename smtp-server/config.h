//
// Created by Karpukhin Aleksandr on 15.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_CONFIG_H
#define NETWORK_PROTOCOLS_CP_CONFIG_H

#define CONFIG_SERVER "server"
#define CONFIG_HOST "host"
#define CONFIG_PORT "port"
#define CONFIG_USER "user"
#define CONFIG_GROUP "group"
#define CONFIG_PROC_COUNT "proc_count"
#define CONFIG_BACKLOG "backlog"
#define CONFIG_DOMAIN "domain"
#define CONFIG_MAILDIR "maildir"

typedef struct {
    const char *host;
    int port;

    const char *user;
    const char *group;

    int proc_count;
    int backlog;

    const char *domain;
    const char *maildir;
} server_config_t;

void parse_app_config(server_config_t *server_config, int argc, char *argv[]);

#endif //NETWORK_PROTOCOLS_CP_CONFIG_H
