#include "simple_utcd/utc_packet.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace simple_utcd {

UTCPacket::UTCPacket() : timestamp_(0) {
    timestamp_ = get_current_utc_timestamp();
}

UTCPacket::UTCPacket(uint32_t timestamp) : timestamp_(timestamp) {
}

UTCPacket::~UTCPacket() {
    // Nothing to clean up
}

bool UTCPacket::from_bytes(const std::vector<uint8_t>& data) {
    if (data.size() < get_packet_size()) {
        return false;
    }
    
    // UTC protocol uses a simple 32-bit timestamp
    // Network byte order (big-endian)
    timestamp_ = (static_cast<uint32_t>(data[0]) << 24) |
                 (static_cast<uint32_t>(data[1]) << 16) |
                 (static_cast<uint32_t>(data[2]) << 8) |
                 static_cast<uint32_t>(data[3]);
    
    return is_valid();
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
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    return static_cast<uint32_t>(time_t);
}

std::string UTCPacket::timestamp_to_string(uint32_t timestamp) {
    std::time_t time_t = static_cast<std::time_t>(timestamp);
    std::tm* tm = std::gmtime(&time_t);
    
    if (!tm) {
        return "Invalid timestamp";
    }
    
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S UTC");
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
    
    // Convert to UTC timestamp
    std::time_t time_t = std::mktime(&tm);
    if (time_t == -1) {
        return 0;
    }
    
    return static_cast<uint32_t>(time_t);
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
    // Basic validation: timestamp should be reasonable
    // Not before 1970 (Unix epoch) and not too far in the future
    
    const uint32_t unix_epoch = 0; // January 1, 1970
    const uint32_t max_reasonable = 4294967295U; // Max uint32_t
    
    // Check if timestamp is within reasonable bounds
    if (timestamp < unix_epoch) {
        return false;
    }
    
    // Check if timestamp is not too far in the future (e.g., 100 years from now)
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    uint32_t current_timestamp = static_cast<uint32_t>(now_time_t);
    
    // Allow some tolerance for clock differences (e.g., 1 hour)
    const uint32_t tolerance = 3600; // 1 hour in seconds
    
    if (timestamp > (current_timestamp + tolerance)) {
        return false;
    }
    
    return true;
}

} // namespace simple_utcd
