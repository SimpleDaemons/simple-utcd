#include "simple_utcd/utc_config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace simple_utcd {

UTCConfig::UTCConfig() {
    set_defaults();
}

UTCConfig::~UTCConfig() {
    // Nothing to clean up
}

void UTCConfig::set_defaults() {
    // Network Configuration
    listen_address_ = "0.0.0.0";
    listen_port_ = 37;  // UTC protocol port
    enable_ipv6_ = true;
    max_connections_ = 1000;
    
    // UTC Server Configuration
    stratum_ = 2;
    reference_id_ = "UTC";
    reference_clock_ = "UTC";
    upstream_servers_ = {"time.nist.gov", "time.google.com", "pool.ntp.org"};
    sync_interval_ = 64;
    timeout_ = 1000;
    
    // Logging Configuration
    log_file_ = "/var/log/simple-utcd/simple-utcd.log";
    log_level_ = "INFO";
    enable_console_logging_ = true;
    enable_syslog_ = false;
    
    // Security Configuration
    enable_authentication_ = false;
    authentication_key_ = "";
    restrict_queries_ = false;
    allowed_clients_ = {};
    denied_clients_ = {};
    
    // Performance Configuration
    worker_threads_ = 4;
    max_packet_size_ = 1024;
    enable_statistics_ = true;
    stats_interval_ = 60;
}

bool UTCConfig::load(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    int line_number = 0;
    
    while (std::getline(file, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Parse configuration line
        if (!parse_config_line(line)) {
            // Log error but continue parsing
            continue;
        }
    }
    
    file.close();
    return true;
}

bool UTCConfig::save(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        return false;
    }
    
    file << "# Simple UTC Daemon Configuration File\n";
    file << "# Generated automatically\n\n";
    
    // Network Configuration
    file << "# Network Configuration\n";
    file << "listen_address = " << listen_address_ << "\n";
    file << "listen_port = " << listen_port_ << "\n";
    file << "enable_ipv6 = " << (enable_ipv6_ ? "true" : "false") << "\n";
    file << "max_connections = " << max_connections_ << "\n\n";
    
    // UTC Server Configuration
    file << "# UTC Server Configuration\n";
    file << "stratum = " << stratum_ << "\n";
    file << "reference_id = " << reference_id_ << "\n";
    file << "reference_clock = " << reference_clock_ << "\n";
    file << "upstream_servers = [";
    for (size_t i = 0; i < upstream_servers_.size(); ++i) {
        if (i > 0) file << ", ";
        file << "\"" << upstream_servers_[i] << "\"";
    }
    file << "]\n";
    file << "sync_interval = " << sync_interval_ << "\n";
    file << "timeout = " << timeout_ << "\n\n";
    
    // Logging Configuration
    file << "# Logging Configuration\n";
    file << "log_file = " << log_file_ << "\n";
    file << "log_level = " << log_level_ << "\n";
    file << "enable_console_logging = " << (enable_console_logging_ ? "true" : "false") << "\n";
    file << "enable_syslog = " << (enable_syslog_ ? "true" : "false") << "\n\n";
    
    // Security Configuration
    file << "# Security Configuration\n";
    file << "enable_authentication = " << (enable_authentication_ ? "true" : "false") << "\n";
    file << "authentication_key = " << authentication_key_ << "\n";
    file << "restrict_queries = " << (restrict_queries_ ? "true" : "false") << "\n";
    file << "allowed_clients = [";
    for (size_t i = 0; i < allowed_clients_.size(); ++i) {
        if (i > 0) file << ", ";
        file << "\"" << allowed_clients_[i] << "\"";
    }
    file << "]\n";
    file << "denied_clients = [";
    for (size_t i = 0; i < denied_clients_.size(); ++i) {
        if (i > 0) file << ", ";
        file << "\"" << denied_clients_[i] << "\"";
    }
    file << "]\n\n";
    
    // Performance Configuration
    file << "# Performance Configuration\n";
    file << "worker_threads = " << worker_threads_ << "\n";
    file << "max_packet_size = " << max_packet_size_ << "\n";
    file << "enable_statistics = " << (enable_statistics_ ? "true" : "false") << "\n";
    file << "stats_interval = " << stats_interval_ << "\n\n";
    
    file.close();
    return true;
}

bool UTCConfig::parse_config_line(const std::string& line) {
    // Find the equals sign
    size_t eq_pos = line.find('=');
    if (eq_pos == std::string::npos) {
        return false;
    }
    
    std::string key = trim(line.substr(0, eq_pos));
    std::string value = trim(line.substr(eq_pos + 1));
    
    // Convert key to lowercase for case-insensitive matching
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    
    // Parse different configuration options
    if (key == "listen_address") {
        listen_address_ = value;
    } else if (key == "listen_port") {
        listen_port_ = std::stoi(value);
    } else if (key == "enable_ipv6") {
        enable_ipv6_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "max_connections") {
        max_connections_ = std::stoi(value);
    } else if (key == "stratum") {
        stratum_ = std::stoi(value);
    } else if (key == "reference_id") {
        reference_id_ = value;
    } else if (key == "reference_clock") {
        reference_clock_ = value;
    } else if (key == "upstream_servers") {
        upstream_servers_ = parse_list(value);
    } else if (key == "sync_interval") {
        sync_interval_ = std::stoi(value);
    } else if (key == "timeout") {
        timeout_ = std::stoi(value);
    } else if (key == "log_file") {
        log_file_ = value;
    } else if (key == "log_level") {
        log_level_ = value;
    } else if (key == "enable_console_logging") {
        enable_console_logging_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "enable_syslog") {
        enable_syslog_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "enable_authentication") {
        enable_authentication_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "authentication_key") {
        authentication_key_ = value;
    } else if (key == "restrict_queries") {
        restrict_queries_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "allowed_clients") {
        allowed_clients_ = parse_list(value);
    } else if (key == "denied_clients") {
        denied_clients_ = parse_list(value);
    } else if (key == "worker_threads") {
        worker_threads_ = std::stoi(value);
    } else if (key == "max_packet_size") {
        max_packet_size_ = std::stoi(value);
    } else if (key == "enable_statistics") {
        enable_statistics_ = (value == "true" || value == "1" || value == "yes");
    } else if (key == "stats_interval") {
        stats_interval_ = std::stoi(value);
    } else {
        // Unknown configuration option
        return false;
    }
    
    return true;
}

std::string UTCConfig::trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos) {
        return "";
    }
    
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> UTCConfig::parse_list(const std::string& str) {
    std::vector<std::string> result;
    
    // Remove brackets if present
    std::string cleaned = str;
    if (cleaned.front() == '[' && cleaned.back() == ']') {
        cleaned = cleaned.substr(1, cleaned.length() - 2);
    }
    
    // Split by comma
    std::stringstream ss(cleaned);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        item = trim(item);
        
        // Remove quotes if present
        if (item.front() == '"' && item.back() == '"') {
            item = item.substr(1, item.length() - 2);
        }
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

} // namespace simple_utcd
