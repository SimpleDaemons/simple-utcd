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
