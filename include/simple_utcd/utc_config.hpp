#pragma once

#include <string>
#include <vector>
#include <memory>

namespace simple_utcd {

class UTCConfig {
public:
    UTCConfig();
    ~UTCConfig();

    bool load(const std::string& config_file);
    bool save(const std::string& config_file);

    // Network Configuration
    const std::string& get_listen_address() const { return listen_address_; }
    int get_listen_port() const { return listen_port_; }
    bool is_ipv6_enabled() const { return enable_ipv6_; }
    int get_max_connections() const { return max_connections_; }

    void set_listen_address(const std::string& address) { listen_address_ = address; }
    void set_listen_port(int port) { listen_port_ = port; }
    void set_ipv6_enabled(bool enabled) { enable_ipv6_ = enabled; }
    void set_max_connections(int max) { max_connections_ = max; }

    // UTC Server Configuration
    int get_stratum() const { return stratum_; }
    const std::string& get_reference_id() const { return reference_id_; }
    const std::string& get_reference_clock() const { return reference_clock_; }
    const std::vector<std::string>& get_upstream_servers() const { return upstream_servers_; }
    int get_sync_interval() const { return sync_interval_; }
    int get_timeout() const { return timeout_; }

    void set_stratum(int stratum) { stratum_ = stratum; }
    void set_reference_id(const std::string& id) { reference_id_ = id; }
    void set_reference_clock(const std::string& clock) { reference_clock_ = clock; }
    void set_upstream_servers(const std::vector<std::string>& servers) { upstream_servers_ = servers; }
    void set_sync_interval(int interval) { sync_interval_ = interval; }
    void set_timeout(int timeout) { timeout_ = timeout; }

    // Logging Configuration
    const std::string& get_log_file() const { return log_file_; }
    const std::string& get_log_level() const { return log_level_; }
    bool is_console_logging_enabled() const { return enable_console_logging_; }
    bool is_syslog_enabled() const { return enable_syslog_; }

    void set_log_file(const std::string& file) { log_file_ = file; }
    void set_log_level(const std::string& level) { log_level_ = level; }
    void set_console_logging_enabled(bool enabled) { enable_console_logging_ = enabled; }
    void set_syslog_enabled(bool enabled) { enable_syslog_ = enabled; }

    // Security Configuration
    bool is_authentication_enabled() const { return enable_authentication_; }
    const std::string& get_authentication_key() const { return authentication_key_; }
    bool is_query_restriction_enabled() const { return restrict_queries_; }
    const std::vector<std::string>& get_allowed_clients() const { return allowed_clients_; }
    const std::vector<std::string>& get_denied_clients() const { return denied_clients_; }

    void set_authentication_enabled(bool enabled) { enable_authentication_ = enabled; }
    void set_authentication_key(const std::string& key) { authentication_key_ = key; }
    void set_query_restriction_enabled(bool enabled) { restrict_queries_ = enabled; }
    void set_allowed_clients(const std::vector<std::string>& clients) { allowed_clients_ = clients; }
    void set_denied_clients(const std::vector<std::string>& clients) { denied_clients_ = clients; }

    // Performance Configuration
    int get_worker_threads() const { return worker_threads_; }
    int get_max_packet_size() const { return max_packet_size_; }
    bool is_statistics_enabled() const { return enable_statistics_; }
    int get_stats_interval() const { return stats_interval_; }

    void set_worker_threads(int threads) { worker_threads_ = threads; }
    void set_max_packet_size(int size) { max_packet_size_ = size; }
    void set_statistics_enabled(bool enabled) { enable_statistics_ = enabled; }
    void set_stats_interval(int interval) { stats_interval_ = interval; }

private:
    // Network Configuration
    std::string listen_address_;
    int listen_port_;
    bool enable_ipv6_;
    int max_connections_;

    // UTC Server Configuration
    int stratum_;
    std::string reference_id_;
    std::string reference_clock_;
    std::vector<std::string> upstream_servers_;
    int sync_interval_;
    int timeout_;

    // Logging Configuration
    std::string log_file_;
    std::string log_level_;
    bool enable_console_logging_;
    bool enable_syslog_;

    // Security Configuration
    bool enable_authentication_;
    std::string authentication_key_;
    bool restrict_queries_;
    std::vector<std::string> allowed_clients_;
    std::vector<std::string> denied_clients_;

    // Performance Configuration
    int worker_threads_;
    int max_packet_size_;
    bool enable_statistics_;
    int stats_interval_;

    void set_defaults();
    bool parse_config_line(const std::string& line);
    std::string trim(const std::string& str);
    std::vector<std::string> parse_list(const std::string& str);
};

} // namespace simple_utcd
