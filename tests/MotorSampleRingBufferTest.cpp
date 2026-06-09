#include "store/MotorSampleRingBuffer.h"

#include <QTest>

class MotorSampleRingBufferTest : public QObject
{
    Q_OBJECT

private slots:
    void startsEmpty();
    void preservesInsertionOrderBeforeCapacity();
    void wrapsAndDropsOldestValues();
    void orderedValuesAfterPartialFill();
};

void MotorSampleRingBufferTest::startsEmpty()
{
    MotorSampleRingBuffer buffer(4);

    QCOMPARE(buffer.size(), 0);
    QCOMPARE(buffer.capacity(), 4);
    QVERIFY(buffer.orderedValues().isEmpty());
}

void MotorSampleRingBufferTest::preservesInsertionOrderBeforeCapacity()
{
    MotorSampleRingBuffer buffer(4);

    buffer.push(10.0);
    buffer.push(20.0);
    buffer.push(30.0);

    QCOMPARE(buffer.size(), 3);
    QCOMPARE(buffer.orderedValues(), QVector<double>({10.0, 20.0, 30.0}));
}

void MotorSampleRingBufferTest::wrapsAndDropsOldestValues()
{
    MotorSampleRingBuffer buffer(3);

    buffer.push(1.0);
    buffer.push(2.0);
    buffer.push(3.0);
    buffer.push(4.0);

    QCOMPARE(buffer.size(), 3);
    QCOMPARE(buffer.orderedValues(), QVector<double>({2.0, 3.0, 4.0}));
}

void MotorSampleRingBufferTest::orderedValuesAfterPartialFill()
{
    MotorSampleRingBuffer buffer(5);

    for (int i = 1; i <= 7; ++i) {
        buffer.push(static_cast<double>(i));
    }

    QCOMPARE(buffer.size(), 5);
    QCOMPARE(buffer.orderedValues(), QVector<double>({3.0, 4.0, 5.0, 6.0, 7.0}));
}

QTEST_MAIN(MotorSampleRingBufferTest)
#include "MotorSampleRingBufferTest.moc"
