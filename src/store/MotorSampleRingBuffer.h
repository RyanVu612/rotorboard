#pragma once

#include <QVector>

class MotorSampleRingBuffer
{
public:
    static constexpr int kDefaultCapacity = 120;

    explicit MotorSampleRingBuffer(int capacity = kDefaultCapacity);

    void push(double value);
    QVector<double> orderedValues() const;

    int size() const { return m_count; }
    int capacity() const { return m_capacity; }

private:
    QVector<double> m_values;
    int m_capacity = 0;
    int m_writeIndex = 0;
    int m_count = 0;
};
