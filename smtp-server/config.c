//
// Created by Karpukhin Aleksandr on 15.02.2022.
//

#include "config.h"
#include "error.h"
#include "defines.h"

err_code_t parse_server_config(server_config_t *server_config, const config_t *config) {
    config_setting_t *server = config_lookup(config, CONFIG_SERVER);

    if (server == NULL) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nserver config loaded\n");

    if (config_setting_lookup_string(server, CONFIG_HOST, &server_config->host) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nhost address loaded\n");

    if (config_setting_lookup_int(server, CONFIG_PORT, &server_config->port) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }
    if (server_config->port < 0 || server_config->port > 65535) {
        return ERR_PARSE_PORT;
    }

    printf("\nport number loaded\n");

    if (config_setting_lookup_string(server, CONFIG_USER, &server_config->user) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nuser name loaded\n");

    if (config_setting_lookup_string(server, CONFIG_GROUP, &server_config->group) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\ngroup name loaded\n");

    if (config_setting_lookup_int(server, CONFIG_BACKLOG, &server_config->backlog) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }
    if (server_config->backlog <= 0) {
        return ERR_PARSE_BACKLOG;
    }

    printf("\nbacklog length loaded\n");

    if (config_setting_lookup_string(server, CONFIG_DOMAIN, &server_config->domain) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\ndomain name loaded\n");

    if (config_setting_lookup_string(server, CONFIG_MAILDIR, &server_config->maildir) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nmaildir path loaded\n");

    if (config_setting_lookup_string(server, CONFIG_QUEUE_PATH, &server_config->queue_path) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nlog queue path loaded\n");

    if (config_setting_lookup_string(server, CONFIG_QUEUE_ID, &server_config->queue_id) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nlog queue id loaded\n");

    if (config_setting_lookup_string(server, CONFIG_LOGDIR, &server_config->log_dir) != CONFIG_TRUE) {
        return ERR_PARSE_CONFIG;
    }

    printf("\nlog_dir path loaded\n");

    return OP_SUCCESS;
}

void parse_app_config(server_config_t *server_config, int argc, char *argv[]) {
    if (server_config == NULL) {
        exit_with_error_code(ERR_NULL_POINTER);
    }

    config_t config;
    char *filename = argc > 1 ? argv[1] : DEFAULT_CONFIG_PATH;
    err_code_t ec;

    config_init(&config);
    if (config_read_file(&config, filename) != CONFIG_TRUE) {
        exit_with_config_error(&config, filename);
    }

    printf("\napp config loaded\n");

    switch (ec = parse_server_config(server_config, &config)) {
        case OP_SUCCESS: break;
        case ERR_PARSE_CONFIG: exit_with_config_error(&config, filename);
        default: exit_with_error_code(ec);
    }

    printf("\napp config parsed\n");

    config_destroy(&config);
}