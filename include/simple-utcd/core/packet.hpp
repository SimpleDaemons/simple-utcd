/*
 * includes/simple_utcd/utc_packet.hpp
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

#include <cstdint>
#include <string>
#include <vector>

namespace simple_utcd {

class UTCPacket {
public:
    UTCPacket();
    UTCPacket(uint32_t timestamp);
    ~UTCPacket();

    // Packet creation and parsing
    bool from_bytes(const std::vector<uint8_t>& data);
    std::vector<uint8_t> to_bytes() const;

    // UTC time handling
    uint32_t get_timestamp() const { return timestamp_; }
    void set_timestamp(uint32_t timestamp) { timestamp_ = timestamp; }
    uint32_t get_timestamp_microseconds() const { return timestamp_microseconds_; }
    void set_timestamp_microseconds(uint32_t microseconds) { timestamp_microseconds_ = microseconds; }
    uint8_t get_version() const { return version_; }
    uint8_t get_mode() const { return mode_; }

    // RFC 868 counts seconds since 1900-01-01 00:00:00 UTC.
    static constexpr uint32_t kSecondsBetween1900And1970 = 2208988800u;
    static uint32_t unix_to_rfc868(uint32_t unix_seconds) {
        return static_cast<uint32_t>(static_cast<uint64_t>(unix_seconds) +
                                     kSecondsBetween1900And1970);
    }
    static uint32_t rfc868_to_unix(uint32_t rfc868_seconds) {
        return static_cast<uint32_t>(static_cast<uint64_t>(rfc868_seconds) -
                                     kSecondsBetween1900And1970);
    }

    // Current time utilities. Values are RFC 868 timestamps.
    static uint32_t get_current_utc_timestamp();
    static std::pair<uint32_t, uint32_t> get_current_utc_timestamp_with_microseconds();
    static std::string timestamp_to_string(uint32_t timestamp);
    static std::string timestamp_to_string_with_microseconds(uint32_t timestamp, uint32_t microseconds);
    static uint32_t string_to_timestamp(const std::string& time_str);
    
    // Leap second handling
    static bool is_leap_second(uint32_t timestamp);
    static int get_leap_second_offset(uint32_t timestamp);

    // Validation
    bool is_valid() const;
    size_t get_packet_size() const;
    bool validate_packet_size(size_t size) const;
    bool validate_checksum(const std::vector<uint8_t>& data) const;
    bool validate_version(uint8_t version) const;
    bool validate_mode(uint8_t mode) const;

    // Debugging
    std::string to_string() const;

private:
    uint32_t timestamp_;           // RFC 868 seconds since 1900-01-01 UTC
    uint32_t timestamp_microseconds_; // Microseconds fraction (0-999999)
    uint8_t version_;               // Protocol version
    uint8_t mode_;                  // Packet mode

    bool validate_timestamp(uint32_t timestamp) const;
    bool validate_microseconds(uint32_t microseconds) const;
    uint16_t calculate_checksum(const std::vector<uint8_t>& data) const;
};

} // namespace simple_utcd
