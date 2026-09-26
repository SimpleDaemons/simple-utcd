/*
 * src/core/utc_packet.cpp
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

#include "simple-utcd/core/packet.hpp"
#include "simple-utcd/utils/error_handler.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace simple_utcd {

UTCPacket::UTCPacket() : timestamp_(0), timestamp_microseconds_(0), version_(1), mode_(3) {
    auto time_pair = get_current_utc_timestamp_with_microseconds();
    timestamp_ = time_pair.first;
    timestamp_microseconds_ = time_pair.second;
}

UTCPacket::UTCPacket(uint32_t timestamp) : timestamp_(timestamp), timestamp_microseconds_(0), version_(1), mode_(3) {
}

UTCPacket::~UTCPacket() {
    // Nothing to clean up
}

bool UTCPacket::from_bytes(const std::vector<uint8_t>& data) {
    // Enhanced validation: check packet size first
    if (!validate_packet_size(data.size())) {
        UTC_ERROR("UTCPacket", "Invalid packet size: expected " + std::to_string(get_packet_size()) +
                  " bytes, got " + std::to_string(data.size()));
        return false;
    }

    // For basic UTC protocol (4 bytes), parse timestamp
    // Network byte order (big-endian)
    timestamp_ = (static_cast<uint32_t>(data[0]) << 24) |
                 (static_cast<uint32_t>(data[1]) << 16) |
                 (static_cast<uint32_t>(data[2]) << 8) |
                 static_cast<uint32_t>(data[3]);

    // For extended packets (future), parse version and mode
    if (data.size() >= 6) {
        version_ = data[4];
        mode_ = data[5];
        
        // Validate version
        if (!validate_version(version_)) {
            UTC_ERROR("UTCPacket", "Invalid protocol version: " + std::to_string(version_));
            return false;
        }
        
        // Validate mode
        if (!validate_mode(mode_)) {
            UTC_ERROR("UTCPacket", "Invalid packet mode: " + std::to_string(mode_));
            return false;
        }
    }

    // Validate checksum if present (for extended packets)
    if (data.size() >= 8 && !validate_checksum(data)) {
        UTC_ERROR("UTCPacket", "Checksum validation failed");
        return false;
    }

    // Validate timestamp
    if (!is_valid()) {
        UTC_ERROR("UTCPacket", "Invalid timestamp in packet: " + std::to_string(timestamp_));
        return false;
    }

    return true;
}

std::vector<uint8_t> UTCPacket::to_bytes() const {
    std::vector<uint8_t> data(get_packet_size());

    // Convert timestamp to network byte order (big-endian)
    data[0] = static_cast<uint8_t>((timestamp_ >> 24) & 0xFF);
    data[1] = static_cast<uint8_t>((timestamp_ >> 16) & 0xFF);
    data[2] = static_cast<uint8_t>((timestamp_ >> 8) & 0xFF);
    data[3] = static_cast<uint8_t>(timestamp_ & 0xFF);

    return data;
}

uint32_t UTCPacket::get_current_utc_timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto unix_sec = static_cast<uint64_t>(std::chrono::system_clock::to_time_t(now));
    return static_cast<uint32_t>(unix_sec + kSecondsBetween1900And1970);
}

std::pair<uint32_t, uint32_t> UTCPacket::get_current_utc_timestamp_with_microseconds() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto timestamp_sec = static_cast<uint32_t>(time_t);
    
    // Get microseconds
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration - seconds);
    
    return std::make_pair(timestamp_sec, static_cast<uint32_t>(microseconds.count() % 1000000));
}

std::string UTCPacket::timestamp_to_string(uint32_t timestamp) {
    const auto unix_sec = static_cast<int64_t>(timestamp) -
                          static_cast<int64_t>(kSecondsBetween1900And1970);
    std::time_t time_t = static_cast<std::time_t>(unix_sec);
    std::tm* tm = std::gmtime(&time_t);

    if (!tm) {
        return "Invalid timestamp";
    }

    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S UTC");
    return ss.str();
}

std::string UTCPacket::timestamp_to_string_with_microseconds(uint32_t timestamp, uint32_t microseconds) {
    const auto unix_sec = static_cast<int64_t>(timestamp) -
                          static_cast<int64_t>(kSecondsBetween1900And1970);
    std::time_t time_t = static_cast<std::time_t>(unix_sec);
    std::tm* tm = std::gmtime(&time_t);

    if (!tm) {
        return "Invalid timestamp";
    }

    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(6) << microseconds;
    ss << " UTC";
    return ss.str();
}

uint32_t UTCPacket::string_to_timestamp(const std::string& time_str) {
    // Parse format: "YYYY-MM-DD HH:MM:SS" or "YYYY-MM-DD HH:MM:SS UTC"
    std::string cleaned = time_str;

    // Remove "UTC" suffix if present
    if (cleaned.length() > 4 && cleaned.substr(cleaned.length() - 4) == " UTC") {
        cleaned = cleaned.substr(0, cleaned.length() - 4);
    }

    std::tm tm = {};
    std::istringstream ss(cleaned);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    if (ss.fail()) {
        return 0;
    }

    // Convert to an RFC 868 timestamp. timegm interprets the broken-down time as UTC.
#if defined(_WIN32)
    std::time_t unix_time = _mkgmtime(&tm);
#else
    std::time_t unix_time = timegm(&tm);
#endif
    if (unix_time < 0) {
        return 0;
    }

    return unix_to_rfc868(static_cast<uint32_t>(unix_time));
}

bool UTCPacket::is_valid() const {
    return validate_timestamp(timestamp_);
}

size_t UTCPacket::get_packet_size() const {
    return 4; // 32-bit timestamp in bytes
}

std::string UTCPacket::to_string() const {
    std::stringstream ss;
    ss << "UTCPacket{timestamp=" << timestamp_
       << ", time=" << timestamp_to_string(timestamp_)
       << ", valid=" << (is_valid() ? "true" : "false") << "}";
    return ss.str();
}

bool UTCPacket::validate_timestamp(uint32_t timestamp) const {
    // Zero is 1900-01-01 in RFC 868 and is not a useful server timestamp.
    // Any other 32-bit value is a legal encoding; the counter wraps in 2036.
    return timestamp != 0;
}

bool UTCPacket::validate_packet_size(size_t size) const {
    // Basic UTC packet is 4 bytes (timestamp only)
    // Extended packets can be 6+ bytes (with version/mode/checksum)
    return size >= get_packet_size() && size <= 48; // Max reasonable packet size
}

bool UTCPacket::validate_checksum(const std::vector<uint8_t>& data) const {
    if (data.size() < 8) {
        return true; // No checksum for basic packets
    }
    
    // Extract stored checksum (last 2 bytes)
    uint16_t stored_checksum = (static_cast<uint16_t>(data[data.size() - 2]) << 8) |
                               static_cast<uint16_t>(data[data.size() - 1]);
    
    // Calculate checksum for data (excluding checksum bytes)
    std::vector<uint8_t> data_for_checksum(data.begin(), data.end() - 2);
    uint16_t calculated_checksum = calculate_checksum(data_for_checksum);
    
    return stored_checksum == calculated_checksum;
}

uint16_t UTCPacket::calculate_checksum(const std::vector<uint8_t>& data) const {
    // Simple checksum: sum of all bytes
    uint32_t sum = 0;
    for (uint8_t byte : data) {
        sum += byte;
    }
    // Return 16-bit checksum
    return static_cast<uint16_t>(sum & 0xFFFF);
}

bool UTCPacket::validate_version(uint8_t version) const {
    // Supported protocol versions: 1-4
    return version >= 1 && version <= 4;
}

bool UTCPacket::validate_mode(uint8_t mode) const {
    // Valid modes: 0-7 (3 bits)
    // Mode 3 = client request, Mode 4 = server response
    return mode <= 7;
}

bool UTCPacket::validate_microseconds(uint32_t microseconds) const {
    // Microseconds must be in range 0-999999
    return microseconds <= 999999;
}

bool UTCPacket::is_leap_second(uint32_t timestamp) {
    // Leap seconds are inserted at the end of June 30 or December 31.
    // RFC 868 itself does not carry a leap indicator; this is advisory only.
    const auto unix_sec = static_cast<int64_t>(timestamp) -
                          static_cast<int64_t>(kSecondsBetween1900And1970);
    std::time_t time_t = static_cast<std::time_t>(unix_sec);
    std::tm* tm = std::gmtime(&time_t);
    
    if (!tm) {
        return false;
    }
    
    // Check if it's June 30 23:59:60 or December 31 23:59:60
    // Note: This is a basic check. Real implementation needs IERS leap second table
    bool is_june_30 = (tm->tm_mon == 5 && tm->tm_mday == 30 && tm->tm_hour == 23 && 
                       tm->tm_min == 59 && tm->tm_sec == 60);
    bool is_dec_31 = (tm->tm_mon == 11 && tm->tm_mday == 31 && tm->tm_hour == 23 && 
                      tm->tm_min == 59 && tm->tm_sec == 60);
    
    return is_june_30 || is_dec_31;
}

int UTCPacket::get_leap_second_offset(uint32_t timestamp) {
    // Get the cumulative leap second offset for a given timestamp
    // This is a simplified implementation - full version would use IERS data
    
    // Known leap seconds (simplified list - should be updated from IERS)
    // Format: (timestamp, offset)
    // This is a placeholder - real implementation needs full IERS table
    static const std::vector<std::pair<uint32_t, int>> leap_seconds = {
        // Add known leap second dates here
        // Example: {946684800, 1}, // 2000-01-01
    };
    
    int offset = 0;
    for (const auto& leap : leap_seconds) {
        if (timestamp >= leap.first) {
            offset = leap.second;
        } else {
            break;
        }
    }
    
    return offset;
}

} // namespace simple_utcd
