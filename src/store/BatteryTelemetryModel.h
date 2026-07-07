#pragma once

#include "model/BatteryTelemetry.h"
#include "model/WarningLevel.h"
#include "store/MotorSampleRingBuffer.h"
#include "warnings/BatteryWarningEvaluator.h"

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

class BatteryTelemetryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int sampleRevision READ sampleRevision NOTIFY sampleRevisionChanged)

public:
    enum Role {
        BatteryIdRole = Qt::UserRole + 1,
        VoltageRole,
        CurrentRole,
        BatteryRemainingRole,
        TemperatureCelsiusRole,
        CurrentConsumedMahRole,
        CellVoltagesRole,
        TimestampMillisRole,
        IsStaleRole,
        WarningLevelRole,
        VoltageHistoryRole,
        CurrentHistoryRole,
        BatteryRemainingHistoryRole,
        TemperatureHistoryRole,
        CurrentConsumedMahHistoryRole
    };
    Q_ENUM(Role)

    explicit BatteryTelemetryModel(QObject *parent = nullptr);
    int sampleRevision() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateTelemetry(const BatteryTelemetry &telemetry);
    void refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis);
    void clear();

    Q_INVOKABLE QVariant valueForMetric(int batteryId, const QString &metric) const;
    Q_INVOKABLE QVariantList historyForMetric(int batteryId, const QString &metric) const;
    Q_INVOKABLE QVariantList cellVoltagesForBattery(int batteryId) const;
    Q_INVOKABLE int rowForBatteryId(int batteryId) const;
    Q_INVOKABLE bool isBatteryStale(int batteryId) const;
    Q_INVOKABLE int warningLevelForBattery(int batteryId) const;
    Q_INVOKABLE int batteryIdAt(int row) const;

signals:
    void sampleRevisionChanged();

private:
    void bumpSampleRevision();
    struct BatteryRow {
        BatteryTelemetry telemetry;
        MotorSampleRingBuffer voltageHistory;
        MotorSampleRingBuffer currentHistory;
        MotorSampleRingBuffer batteryRemainingHistory;
        MotorSampleRingBuffer temperatureHistory;
        MotorSampleRingBuffer currentConsumedMahHistory;
        bool isStale = false;
        WarningLevel warningLevel = WarningLevel::Ok;
    };
    QVector<int> rolesForSampleUpdate() const;
    QVector<int> rolesForStaleUpdate() const;

    QVector<BatteryRow> m_rows;
    QHash<int, int> m_rowByBatteryId;
    BatteryWarningEvaluator m_warningEvaluator;
    int m_sampleRevision = 0;
};
