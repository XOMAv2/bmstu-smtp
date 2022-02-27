//
// Created by Karpukhin Aleksandr on 15.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_CONFIG_H
#define NETWORK_PROTOCOLS_CP_CONFIG_H

#define CONFIG_SERVER     "server"
#define CONFIG_HOST       "host"
#define CONFIG_PORT       "port"
#define CONFIG_USER       "user"
#define CONFIG_GROUP      "group"
#define CONFIG_BACKLOG    "backlog"
#define CONFIG_DOMAIN     "domain"
#define CONFIG_MAILDIR    "maildir"
#define CONFIG_QUEUE_PATH "queue_path"
#define CONFIG_QUEUE_ID   "queue_id"
#define CONFIG_LOGDIR     "log_dir"

typedef struct {
    const char *host;
    int         port;

    const char *user;
    const char *group;

    int         backlog;

    const char *domain;
    const char *maildir;

    const char *queue_path;
    int         queue_id;
    const char *log_dir;
} server_config_t;

void parse_app_config(server_config_t *server_config, int argc, char *argv[]);

#endif //NETWORK_PROTOCOLS_CP_CONFIG_H
