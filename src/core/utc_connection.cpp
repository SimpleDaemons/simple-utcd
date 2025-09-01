#include "simple_utcd/utc_connection.hpp"
#include "simple_utcd/utc_packet.hpp"
#include "simple_utcd/platform.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace simple_utcd {

UTCConnection::UTCConnection(int socket_fd, const std::string& client_address,
                             UTCConfig* config, Logger* logger)
    : socket_fd_(socket_fd)
    , client_address_(client_address)
    , config_(config)
    , logger_(logger)
    , connected_(true)
    , packets_sent_(0)
    , packets_received_(0)
    , bytes_sent_(0)
    , bytes_received_(0)
{
    if (logger_) {
        logger_->info("New connection from {}", client_address_);
    }
}

UTCConnection::~UTCConnection() {
    close_connection();
}

bool UTCConnection::send_packet(const UTCPacket& packet) {
    if (!connected_) {
        return false;
    }
    
    // Check if client is allowed
    if (!is_client_allowed()) {
        if (logger_) {
            logger_->warn("Connection from {} denied by access control", client_address_);
        }
        return false;
    }
    
    // Convert packet to bytes
    std::vector<uint8_t> data = packet.to_bytes();
    
    // Send packet data
    if (!send_data(data.data(), data.size())) {
        return false;
    }
    
    packets_sent_++;
    bytes_sent_ += data.size();
    
    if (logger_) {
        logger_->debug("Sent UTC packet to {}: {}", client_address_, packet.to_string());
    }
    
    return true;
}

bool UTCConnection::receive_packet(UTCPacket& packet) {
    if (!connected_) {
        return false;
    }
    
    // UTC protocol typically doesn't receive packets from clients
    // This is mainly for completeness and future extensions
    std::vector<uint8_t> data(4); // UTC packet is 4 bytes
    
    if (!receive_data(data.data(), data.size())) {
        return false;
    }
    
    if (!packet.from_bytes(data)) {
        if (logger_) {
            logger_->warn("Invalid packet received from {}", client_address_);
        }
        return false;
    }
    
    packets_received_++;
    bytes_received_ += data.size();
    
    if (logger_) {
        logger_->debug("Received packet from {}: {}", client_address_, packet.to_string());
    }
    
    return true;
}

void UTCConnection::close_connection() {
    if (connected_) {
        connected_ = false;
        
        if (logger_) {
            logger_->info("Closing connection from {} (sent: {}, received: {})", 
                         client_address_, packets_sent_, packets_received_);
        }
        
        Platform::close_socket(socket_fd_);
    }
}

bool UTCConnection::send_data(const void* data, size_t size) {
    if (!connected_) {
        return false;
    }
    
    const char* buffer = static_cast<const char*>(data);
    size_t total_sent = 0;
    
    while (total_sent < size) {
        ssize_t sent = send(socket_fd_, buffer + total_sent, size - total_sent, 0);
        
        if (sent < 0) {
            if (logger_) {
                logger_->error("Failed to send data to {}: {}", 
                              client_address_, Platform::get_last_error());
            }
            connected_ = false;
            return false;
        }
        
        total_sent += sent;
    }
    
    return true;
}

bool UTCConnection::receive_data(void* data, size_t size) {
    if (!connected_) {
        return false;
    }
    
    char* buffer = static_cast<char*>(data);
    size_t total_received = 0;
    
    while (total_received < size) {
        ssize_t received = recv(socket_fd_, buffer + total_received, 
                               size - total_received, 0);
        
        if (received < 0) {
            if (logger_) {
                logger_->error("Failed to receive data from {}: {}", 
                              client_address_, Platform::get_last_error());
            }
            connected_ = false;
            return false;
        } else if (received == 0) {
            // Connection closed by client
            if (logger_) {
                logger_->info("Connection closed by client {}", client_address_);
            }
            connected_ = false;
            return false;
        }
        
        total_received += received;
    }
    
    return true;
}

bool UTCConnection::is_client_allowed() const {
    if (!config_) {
        return true; // No restrictions if no config
    }
    
    // Check if client is in denied list
    if (is_client_denied()) {
        return false;
    }
    
    // If query restriction is enabled, check allowed list
    if (config_->is_query_restriction_enabled()) {
        const auto& allowed_clients = config_->get_allowed_clients();
        if (allowed_clients.empty()) {
            return true; // No restrictions if list is empty
        }
        
        // Check if client is in allowed list
        for (const auto& allowed : allowed_clients) {
            if (client_address_ == allowed) {
                return true;
            }
        }
        
        return false; // Not in allowed list
    }
    
    return true; // No restrictions
}

bool UTCConnection::is_client_denied() const {
    if (!config_) {
        return false; // No restrictions if no config
    }
    
    const auto& denied_clients = config_->get_denied_clients();
    if (denied_clients.empty()) {
        return false; // No restrictions if list is empty
    }
    
    // Check if client is in denied list
    for (const auto& denied : denied_clients) {
        if (client_address_ == denied) {
            return true;
        }
    }
    
    return false; // Not in denied list
}

} // namespace simple_utcd
