#pragma once

#include <string>

namespace simple_utcd {

class Platform {
public:
    // Platform detection
    static bool is_windows();
    static bool is_macos();
    static bool is_linux();
    static std::string get_platform_name();

    // Network utilities
    static int create_socket(int domain, int type, int protocol);
    static int close_socket(int socket_fd);
    static bool set_socket_option(int socket_fd, int level, int option, const void* value, int value_size);
    static bool bind_socket(int socket_fd, const std::string& address, int port);
    static bool listen_socket(int socket_fd, int backlog);
    static int accept_connection(int socket_fd, std::string& client_address);

    // Time utilities
    static uint32_t get_system_time();
    static uint32_t get_utc_time();
    static void sleep_milliseconds(int milliseconds);

    // File system utilities
    static bool file_exists(const std::string& path);
    static bool create_directory(const std::string& path);
    static std::string get_home_directory();
    static std::string get_config_directory();
    static std::string get_log_directory();

    // Process utilities
    static int get_process_id();
    static std::string get_process_name();
    static bool daemonize();

    // Error handling
    static std::string get_last_error();
    static void set_last_error(const std::string& error);

private:
    static std::string last_error_;
};

} // namespace simple_utcd
