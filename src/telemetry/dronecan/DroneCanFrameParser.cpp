#include "DroneCanFrameParser.h"

#include "HobbywingMessages.h"

#include <QDateTime>

namespace {

bool isHobbywingStatusMessage(uint16_t dataTypeId)
{
    return dataTypeId == hobbywing::kStatusMsg1Id ||
           dataTypeId == hobbywing::kStatusMsg2Id ||
           dataTypeId == hobbywing::kStatusMsg3Id;
}

} // namespace

DroneCanFrameParser::DroneCanFrameParser()
{
    canardInit(&m_instance,
               m_memoryPool,
               sizeof(m_memoryPool),
               onTransferReception,
               shouldAcceptTransfer,
               this);
    canardSetLocalNodeID(&m_instance, 127);
}

void DroneCanFrameParser::setMotorCache(DroneCanMotorCache *cache)
{
    m_cache = cache;
}

void DroneCanFrameParser::handleCanFrame(const CanardCANFrame &frame, qint64 timestampMillis)
{
    Q_UNUSED(timestampMillis);
    const uint64_t nowUsec = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch()) * 1000ULL;
    canardHandleRxFrame(&m_instance, &frame, nowUsec);
}

bool DroneCanFrameParser::shouldAcceptTransfer(const CanardInstance *ins,
                                               uint64_t *outDataTypeSignature,
                                               uint16_t dataTypeId,
                                               CanardTransferType transferType,
                                               uint8_t sourceNodeId)
{
    Q_UNUSED(ins);
    Q_UNUSED(sourceNodeId);

    if (transferType != CanardTransferTypeBroadcast || !isHobbywingStatusMessage(dataTypeId)) {
        return false;
    }

    switch (dataTypeId) {
    case hobbywing::kStatusMsg1Id:
        *outDataTypeSignature = hobbywing::kStatusMsg1Signature;
        return true;
    case hobbywing::kStatusMsg2Id:
        *outDataTypeSignature = hobbywing::kStatusMsg2Signature;
        return true;
    case hobbywing::kStatusMsg3Id:
        *outDataTypeSignature = hobbywing::kStatusMsg3Signature;
        return true;
    default:
        return false;
    }
}

void DroneCanFrameParser::onTransferReception(CanardInstance *ins, CanardRxTransfer *transfer)
{
    auto *parser = static_cast<DroneCanFrameParser *>(ins->user_reference);
    if (!parser || !parser->m_cache || !transfer) {
        return;
    }

    const qint64 timestampMillis = QDateTime::currentMSecsSinceEpoch();
    const uint8_t nodeId = transfer->source_node_id;

    if (transfer->data_type_id == hobbywing::kStatusMsg1Id) {
        hobbywing::StatusMsg1 message{};
        if (hobbywing::decodeStatusMsg1(transfer->payload_head, transfer->payload_len, &message)) {
            parser->m_cache->handleStatusMsg1(nodeId, message, timestampMillis);
        }
    } else if (transfer->data_type_id == hobbywing::kStatusMsg2Id) {
        hobbywing::StatusMsg2 message{};
        if (hobbywing::decodeStatusMsg2(transfer->payload_head, transfer->payload_len, &message)) {
            parser->m_cache->handleStatusMsg2(nodeId, message, timestampMillis);
        }
    } else if (transfer->data_type_id == hobbywing::kStatusMsg3Id) {
        hobbywing::StatusMsg3 message{};
        if (hobbywing::decodeStatusMsg3(transfer->payload_head, transfer->payload_len, &message)) {
            parser->m_cache->handleStatusMsg3(nodeId, message, timestampMillis);
        }
    }
}
