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

    if (config_setting_lookup_int(server_settings, CONFIG_SERVER_RETRIES, &server_config->retries) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse server retries number");
        return ERR_PARSE_SERVER_CONFIG;
    }
    printf("\nretries number loaded\n");

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

// Parse logger configuration from config instance and store it in logger_state instance.
err_code_t parse_logger_config(const config_setting_t *logger_settings,
                               logger_config_t *restrict logger_config,
                               char *restrict error_message) {
    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_NAME, &logger_config->name) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger name");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    printf("\nlogger name loaded\n");

    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_LOG_DIR, &logger_config->log_dir) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger log_dir");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    printf("\nlogger log dir loaded\n");

    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_QUEUE_PATH, &logger_config->queue_path) !=
        CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger queue path");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    printf("\nlogger queue path loaded\n");

    if (config_setting_lookup_int(logger_settings, CONFIG_LOGGER_QUEUE_ID, &logger_config->queue_id) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger queue id");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    printf("\nlogger queue id loaded\n");

    const char *buff;
    if (config_setting_lookup_string(logger_settings, CONFIG_LOGGER_LOG_LEVEL, &buff) != CONFIG_TRUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "can't parse logger log level");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    log_level_te log_level = str_to_log_level(buff);
    if (log_level == ERR_UNKNOWN_ENUM_VALUE) {
        snprintf(error_message, ERROR_MESSAGE_MAX_LENGTH, "invalid logger log level value");
        return ERR_PARSE_LOGGER_CONFIG;
    }
    logger_config->log_level = log_level;
    printf("\nlogger log level loaded\n");

    return OP_SUCCESS;
}

// Parse whole application configuration. Config path can be passed through 
// command line arguments, otherwise default path "../server.cfg" will be used.
// Returns config_t instance, that should be destroyed only when server terminates.
// Do not try to destroy config instance before server termination because it will lead
// to releasing all allocated resources and unexpected behaviour of all code that uses application configuration.
config_t parse_app_config(app_config_t * restrict app_config, int argc, char *argv[]) {
    if (app_config == NULL) {
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
        exit_with_error_message(ERR_PARSE_SERVER_CONFIG, "server section not found");
    }
    printf("\nserver config loaded\n");

    const config_setting_t *logger_settings = config_lookup(&config, CONFIG_LOGGER_SECTION);
    if (logger_settings == NULL) {
        exit_with_error_message(ERR_PARSE_LOGGER_CONFIG, "logger section not found");
    }
    printf("\nlogger config loaded\n");

    char error_message[ERROR_MESSAGE_MAX_LENGTH];
    if (parse_server_config(server_settings, &app_config->server_config, error_message) < 0) {
        exit_with_error_message(ERR_PARSE_SERVER_CONFIG, error_message);
    } else if (parse_logger_config(logger_settings, &app_config->logger_config, error_message) < 0) {
        exit_with_error_message(ERR_PARSE_LOGGER_CONFIG, error_message);
    }

    printf("\napp config parsed\n");

    return config;
}

void log_app_config(const app_config_t * restrict app_config)
{
    if (app_config == NULL) {
        log_info("app config is undefined");
    } else {
        log_server_config(&app_config->server_config);
        log_logger_config(&app_config->logger_config);
    }
}

void log_server_config(const server_config_t * restrict server_config)
{
    if (server_config == NULL) {
        log_info("server config is undefined");
    } else {
        log_info("server config:");
        log_info("host: %s", server_config->host);
        log_info("port: %d", server_config->port);
        log_info("user: %s", server_config->user);
        log_info("group: %s", server_config->group);
        log_info("domain: %s", server_config->domain);
        log_info("maildir: %s", server_config->maildir);
        log_info("backlog: %d", server_config->backlog);
        log_info("retries: %d", server_config->retries);
    }
}

void log_logger_config(const logger_config_t * restrict logger_config)
{
    if (logger_config == NULL) {
        log_info("logger config is undefined");
    } else {
        log_info("logger config:");
        log_info("name: %s", logger_config->name);
        log_info("log_dir: %s", logger_config->log_dir);
        log_info("queue_path: %s", logger_config->queue_path);
        log_info("queue_id: %d", logger_config->queue_id);
        log_info("log_level: %s", log_level_to_str(logger_config->log_level));
    }
}
