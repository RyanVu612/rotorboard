#include "MotorSampleRingBuffer.h"

MotorSampleRingBuffer::MotorSampleRingBuffer(int capacity)
    : m_values(capacity, 0.0)
    , m_capacity(capacity)
{
}

void MotorSampleRingBuffer::push(double value)
{
    if (m_capacity <= 0) {
        return;
    }

    m_values[m_writeIndex] = value;
    m_writeIndex = (m_writeIndex + 1) % m_capacity;
    if (m_count < m_capacity) {
        ++m_count;
    }
}

QVector<double> MotorSampleRingBuffer::orderedValues() const
{
    if (m_count == 0) {
        return {};
    }

    QVector<double> ordered;
    ordered.reserve(m_count);

    const int startIndex = m_count < m_capacity ? 0 : m_writeIndex;
    for (int i = 0; i < m_count; ++i) {
        ordered.push_back(m_values[(startIndex + i) % m_capacity]);
    }

    return ordered;
}
