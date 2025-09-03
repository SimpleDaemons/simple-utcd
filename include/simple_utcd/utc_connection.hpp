/*
 * includes/simple_utcd/utc_connection.hpp
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

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include "utc_packet.hpp"

namespace simple_utcd {

class UTCConfig;
class Logger;

class UTCConnection {
public:
    UTCConnection(int socket_fd, const std::string& client_address,
                  UTCConfig* config, Logger* logger);
    ~UTCConnection();

    bool is_connected() const { return connected_; }
    const std::string& get_client_address() const { return client_address_; }
    int get_socket_fd() const { return socket_fd_; }

    bool send_packet(const UTCPacket& packet);
    bool receive_packet(UTCPacket& packet);
    void close_connection();

    // Connection statistics
    int get_packets_sent() const { return packets_sent_; }
    int get_packets_received() const { return packets_received_; }
    int get_bytes_sent() const { return bytes_sent_; }
    int get_bytes_received() const { return bytes_received_; }

private:
    int socket_fd_;
    std::string client_address_;
    UTCConfig* config_;
    Logger* logger_;

    std::atomic<bool> connected_;
    std::atomic<int> packets_sent_;
    std::atomic<int> packets_received_;
    std::atomic<int> bytes_sent_;
    std::atomic<int> bytes_received_;

    bool send_data(const void* data, size_t size);
    bool receive_data(void* data, size_t size);
    bool is_client_allowed() const;
    bool is_client_denied() const;
};

} // namespace simple_utcd
