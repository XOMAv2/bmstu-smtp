//
// Created by Karpukhin Aleksandr on 15.02.2022.
//

#include <assert.h>

#include "config.h"
#include "error.h"
#include "defines.h"

// Parse server configuration from config and store at server_config.
err_code_t parse_server_config(const config_setting_t *server_settings,
                               server_config_t *restrict server_config,
                               char *restrict error_message) {
    assert(server_settings != NULL && server_config != NULL && error_message != NULL);

    if (config_setting_lookup_string(server_settings, CONFIG_SERVER_HOST, &server_config->host) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server host");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\nhost address loaded\n");

    if (config_setting_lookup_int(server_settings, CONFIG_SERVER_PORT, &server_config->port) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server port");
        return ERR_PARSE_SERVER_CONFIG;
    }
    if (server_config->port < 0 || server_config->port > 65535) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "server port must be in range [0, 65353]");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\nport number loaded\n");

    if (config_setting_lookup_string(server_settings, CONFIG_SERVER_USER, &server_config->user) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server user");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\nuser name loaded\n");

    if (config_setting_lookup_string(server_settings, CONFIG_SERVER_GROUP, &server_config->group) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server group");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\ngroup name loaded\n");

    if (config_setting_lookup_int(server_settings, CONFIG_SERVER_BACKLOG, &server_config->backlog) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server backlog length");
        return ERR_PARSE_SERVER_CONFIG;
    }
    if (server_config->backlog <= 0) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "server backlog length must be strictly positive");
        return ERR_PARSE_BACKLOG;
    }
    printf("\nbacklog length loaded\n");

    if (config_setting_lookup_string(server_settings, CONFIG_SERVER_DOMAIN, &server_config->domain) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server domain");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\ndomain name loaded\n");

    if (config_setting_lookup_string(server_settings, CONFIG_SERVER_MAILDIR, &server_config->maildir) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server maildir");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\nmaildir path loaded\n");

    return OP_SUCCESS;
}

log_level_te str_to_log_level(const char *str) {
    if (strcmp(str, "ERROR") == 0) {
        return ERROR;
    } else if (strcmp(str, "WARN") == 0) {
        return WARN;
    } else if (strcmp(str, "INFO") == 0) {
        return INFO;
    } else if (strcmp(str, "DEBUG") == 0) {
        return DEBUG;
    } else if (strcmp(str, "TRACE") == 0) {
        return TRACE;
    }

    return ERR_UNKNOWN_ENUM_VALUE;
}

// Parse logger configuration from config instance and store it in logger_state instance.
err_code_t parse_logger_config(const config_setting_t *logger_settings,
                               logger_state_t *restrict logger_state,
                               char *restrict error_message) {
    const char *buff;
    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_NAME, &buff) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger name");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    strncpy(logger_state->logger_name, buff, LOGGER_NAME_MAX_LENGTH);
    printf("\nlogger name loaded\n");

    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_LOG_DIR, &buff) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger log_dir");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    strncpy(logger_state->logs_dir, buff, LOGGER_PATH_MAX_LENGTH);
    printf("\nlogger log dir loaded\n");

    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_QUEUE_PATH, &buff) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger queue path");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    strncpy(logger_state->fd_queue_path, buff, LOGGER_PATH_MAX_LENGTH);
    printf("\nlogger queue path loaded\n");

    if (config_setting_lookup_int(logger_settings, CONFIG_LOGGER_QUEUE_ID, &logger_state->queue_id) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger queue id");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    printf("\nlogger queue id loaded\n");

    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_LOG_LEVEL, &buff) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger log level");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    log_level_te log_level = str_to_log_level(buff);
    if (log_level == ERR_UNKNOWN_ENUM_VALUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "invalid logger log level value");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    logger_state->current_log_level = log_level;
    printf("\nlogger log level loaded\n");
}

// Parse whole application configuration. Config path can be passed through 
// command line arguments, otherwise default path "../server.cfg" will be used.
void parse_app_config(server_config_t *restrict server_config,
                      logger_state_t *restrict logger_state,
                      int argc,
                      char *argv[]) {
    if (server_config == NULL) {
        exit_with_error_code(ERR_NULL_POINTER);
    }

    config_t config;
    char *filename = argc > 1 ? argv[1] : DEFAULT_CONFIG_PATH;

    config_init(&config);
    if (config_read_file(&config, filename) != CONFIG_TRUE) {
        exit_with_config_error(ERR_PARSE_CONFIG, &config, filename);
    }
    printf("\napp config loaded\n");

    const config_setting_t *server_settings = config_lookup(&config, CONFIG_SERVER_SECTION);
    if (server_settings == NULL) {
        return exit_with_error_message(ERR_PARSE_SERVER_CONFIG, "server section not found");
    }
    printf("\nserver config loaded\n");

    const config_setting_t *logger_settings = config_lookup(&config, CONFIG_LOGGER_SECTION);
    if (logger_settings == NULL) {
        return exit_with_error_message(ERR_PARSE_LOGGER_CONFIG, "logger section not found");
    }
    printf("\nlogger config loaded\n");

    char error_message[ERROR_MESSAGE_MAX_LENGTH];
    if (parse_server_config(server_settings, server_config, error_message) < 0) {
        exit_with_error_message(ERR_PARSE_SERVER_CONFIG, error_message);
    } else if (parse_logger_config(logger_settings, logger_state, error_message) < 0) {
        exit_with_error_message(ERR_PARSE_LOGGER_CONFIG, error_message);
    }

    printf("\napp config parsed\n");

    config_destroy(&config);
}