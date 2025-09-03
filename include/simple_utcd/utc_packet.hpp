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

    // Current time utilities
    static uint32_t get_current_utc_timestamp();
    static std::string timestamp_to_string(uint32_t timestamp);
    static uint32_t string_to_timestamp(const std::string& time_str);

    // Validation
    bool is_valid() const;
    size_t get_packet_size() const;

    // Debugging
    std::string to_string() const;

private:
    uint32_t timestamp_;

    bool validate_timestamp(uint32_t timestamp) const;
};

} // namespace simple_utcd
