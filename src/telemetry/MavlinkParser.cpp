#include "MavlinkParser.h"

std::optional<mavlink_message_t> MavlinkParser::feedByte(uint8_t byte)
{
    if (mavlink_parse_char(kChannel, byte, &m_message, &m_status)) {
        return m_message;
    }

    return std::nullopt;
}

void MavlinkParser::reset()
{
    m_message = {};
    m_status = {};
}
