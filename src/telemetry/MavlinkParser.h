#pragma once

#include <cstdint>
#include <optional>

extern "C" {
#include "mavlink.h"
}

class MavlinkParser
{
public:
    std::optional<mavlink_message_t> feedByte(uint8_t byte);
    void reset();

private:
    mavlink_message_t m_message{};
    mavlink_status_t m_status{};
    static constexpr uint8_t kChannel = 0;
};
