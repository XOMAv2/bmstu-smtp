//
// Created by Karpukhin Aleksandr on 15.02.2022.
//

#ifndef NETWORK_PROTOCOLS_CP_CONFIG_H
#define NETWORK_PROTOCOLS_CP_CONFIG_H

#include <libconfig.h>

#include "../smtp-common/log.h"

#define CONFIG_SERVER_SECTION            "server"
#define CONFIG_LOGGER_SECTION            "logger"

#define CONFIG_SERVER_HOST               "host"
#define CONFIG_SERVER_PORT               "port"
#define CONFIG_SERVER_USER               "user"
#define CONFIG_SERVER_GROUP              "group"
#define CONFIG_SERVER_BACKLOG            "backlog"
#define CONFIG_SERVER_RETRIES            "retries"
#define CONFIG_SERVER_DOMAIN             "domain"
#define CONFIG_SERVER_MAILDIR            "maildir"

#define CONFIG_LOGGER_NAME               "name"
#define CONFIG_LOGGER_LOG_DIR            "log_dir"
#define CONFIG_LOGGER_QUEUE_PATH         "queue_path"
#define CONFIG_LOGGER_QUEUE_ID           "queue_id"
#define CONFIG_LOGGER_LOG_LEVEL          "log_level"

typedef struct {
    const char *host;
    int         port;

    const char *user;
    const char *group;

    int         backlog;
    int         retries;

    const char *domain;
    const char *maildir;

    const char *queue_path;
    int         queue_id;
    const char *log_dir;
} server_config_t;

typedef struct {
    const char *name;
    const char *log_dir;
    const char *queue_path;
    int queue_id;
    log_level_te log_level;
} logger_config_t;

typedef struct {
    server_config_t server_config;
    logger_config_t logger_config;
} app_config_t;

config_t parse_app_config(app_config_t * restrict app_config, int argc, char *argv[]);

void log_app_config(const app_config_t * restrict app_config);
void log_server_config(const server_config_t * restrict server_config);
void log_logger_config(const logger_config_t * restrict logger_config);

#endif //NETWORK_PROTOCOLS_CP_CONFIG_H
