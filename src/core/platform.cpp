/*
 * src/core/platform.cpp
 *
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "simple_utcd/platform.hpp"
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <shlobj.h>
#pragma comment(lib, "ws2_32.lib")
#elif __APPLE__
#include <mach/mach_time.h>
#include <mach/mach.h>
#elif __linux__
#include <sys/time.h>
#include <time.h>
#endif

namespace simple_utcd {

std::string Platform::last_error_;

bool Platform::is_windows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

bool Platform::is_macos() {
#ifdef __APPLE__
    return true;
#else
    return false;
#endif
}

bool Platform::is_linux() {
#ifdef __linux__
    return true;
#else
    return false;
#endif
}

std::string Platform::get_platform_name() {
    if (is_windows()) {
        return "Windows";
    } else if (is_macos()) {
        return "macOS";
    } else if (is_linux()) {
        return "Linux";
    } else {
        return "Unknown";
    }
}

int Platform::create_socket(int domain, int type, int protocol) {
#ifdef _WIN32
    WSADATA wsaData;
    static bool wsa_initialized = false;
    if (!wsa_initialized) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            last_error_ = "WSAStartup failed";
            return -1;
        }
        wsa_initialized = true;
    }
#endif

    int sock = socket(domain, type, protocol);
    if (sock < 0) {
#ifdef _WIN32
        last_error_ = "socket() failed: " + std::to_string(WSAGetLastError());
#else
        last_error_ = "socket() failed: " + std::string(strerror(errno));
#endif
    }
    return sock;
}

int Platform::close_socket(int socket_fd) {
#ifdef _WIN32
    int result = closesocket(socket_fd);
    if (result != 0) {
        last_error_ = "closesocket() failed: " + std::to_string(WSAGetLastError());
    }
    return result;
#else
    int result = close(socket_fd);
    if (result != 0) {
        last_error_ = "close() failed: " + std::string(strerror(errno));
    }
    return result;
#endif
}

bool Platform::set_socket_option(int socket_fd, int level, int option, const void* value, int value_size) {
    int result = setsockopt(socket_fd, level, option,
                           reinterpret_cast<const char*>(value), value_size);
    if (result != 0) {
#ifdef _WIN32
        last_error_ = "setsockopt() failed: " + std::to_string(WSAGetLastError());
#else
        last_error_ = "setsockopt() failed: " + std::string(strerror(errno));
#endif
        return false;
    }
    return true;
}

bool Platform::bind_socket(int socket_fd, const std::string& address, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (address.empty() || address == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, address.c_str(), &addr.sin_addr) <= 0) {
            last_error_ = "Invalid address: " + address;
            return false;
        }
    }

    int result = bind(socket_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
    if (result != 0) {
#ifdef _WIN32
        last_error_ = "bind() failed: " + std::to_string(WSAGetLastError());
#else
        last_error_ = "bind() failed: " + std::string(strerror(errno));
#endif
        return false;
    }
    return true;
}

bool Platform::listen_socket(int socket_fd, int backlog) {
    int result = listen(socket_fd, backlog);
    if (result != 0) {
#ifdef _WIN32
        last_error_ = "listen() failed: " + std::to_string(WSAGetLastError());
#else
        last_error_ = "listen() failed: " + std::string(strerror(errno));
#endif
        return false;
    }
    return true;
}

int Platform::accept_connection(int socket_fd, std::string& client_address) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(socket_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
#ifdef _WIN32
        last_error_ = "accept() failed: " + std::to_string(WSAGetLastError());
#else
        last_error_ = "accept() failed: " + std::string(strerror(errno));
#endif
        return -1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    client_address = std::string(client_ip);

    return client_fd;
}

uint32_t Platform::get_system_time() {
    return static_cast<uint32_t>(std::time(nullptr));
}

uint32_t Platform::get_utc_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return static_cast<uint32_t>(time_t);
}

void Platform::sleep_milliseconds(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

bool Platform::file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool Platform::create_directory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), nullptr) != 0;
#else
    return mkdir(path.c_str(), 0755) == 0;
#endif
}

std::string Platform::get_home_directory() {
#ifdef _WIN32
    const char* user_profile = std::getenv("USERPROFILE");
    if (user_profile) {
        return std::string(user_profile);
    }
    return "";
#else
    const char* home = getenv("HOME");
    if (home) {
        return std::string(home);
    }

    struct passwd* pw = getpwuid(getuid());
    if (pw && pw->pw_dir) {
        return std::string(pw->pw_dir);
    }

    return "";
#endif
}

std::string Platform::get_config_directory() {
    if (is_windows()) {
        const char* app_data = std::getenv("APPDATA");
        if (app_data) {
            return std::string(app_data) + "\\Simple-UTCd";
        }
        return "C:\\ProgramData\\Simple-UTCd";
    } else if (is_macos()) {
        return "/usr/local/etc/simple-utcd";
    } else {
        return "/etc/simple-utcd";
    }
}

std::string Platform::get_log_directory() {
    if (is_windows()) {
        const char* app_data = std::getenv("APPDATA");
        if (app_data) {
            return std::string(app_data) + "\\Simple-UTCd\\logs";
        }
        return "C:\\ProgramData\\Simple-UTCd\\logs";
    } else if (is_macos()) {
        return "/usr/local/var/log/simple-utcd";
    } else {
        return "/var/log/simple-utcd";
    }
}

int Platform::get_process_id() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

std::string Platform::get_process_name() {
#ifdef _WIN32
    char process_name[MAX_PATH];
    if (GetModuleFileNameA(nullptr, process_name, MAX_PATH) != 0) {
        return std::string(process_name);
    }
    return "unknown";
#else
    std::ifstream comm("/proc/self/comm");
    if (comm.is_open()) {
        std::string name;
        std::getline(comm, name);
        return name;
    }
    return "unknown";
#endif
}

bool Platform::daemonize() {
#ifdef _WIN32
    // Windows doesn't have daemon() function
    // This would typically be handled by the Windows Service system
    return true;
#else
    // Modern daemon implementation using fork() instead of deprecated daemon()
    pid_t pid = fork();

    if (pid < 0) {
        return false; // Fork failed
    }

    if (pid > 0) {
        exit(0); // Parent process exits
    }

    // Child process continues
    // Create new session and become session leader
    if (setsid() < 0) {
        return false;
    }

    // Fork again to ensure we're not a session leader
    pid = fork();
    if (pid < 0) {
        return false;
    }

    if (pid > 0) {
        exit(0); // First child exits
    }

    // Second child process continues as daemon
    // Change working directory to root
    chdir("/");

    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Redirect standard file descriptors to /dev/null
    open("/dev/null", O_RDONLY); // stdin
    open("/dev/null", O_WRONLY); // stdout
    open("/dev/null", O_WRONLY); // stderr

    return true;
#endif
}

std::string Platform::get_last_error() {
    return last_error_;
}

void Platform::set_last_error(const std::string& error) {
    last_error_ = error;
}

} // namespace simple_utcd
