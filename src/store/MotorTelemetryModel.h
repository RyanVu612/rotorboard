#pragma once

#include "model/MotorTelemetry.h"
#include "model/WarningLevel.h"
#include "store/MotorSampleRingBuffer.h"
#include "warnings/MotorWarningEvaluator.h"

#include <QAbstractListModel>
#include <QHash>
#include <QVector>

class MotorTelemetryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int sampleRevision READ sampleRevision NOTIFY sampleRevisionChanged)

public:
    enum Role {
        MotorIdRole = Qt::UserRole + 1,
        RpmRole,
        VoltageRole,
        CurrentRole,
        TemperatureCelsiusRole,
        PwmRole,
        StatusRole,
        TimestampMillisRole,
        IsStaleRole,
        WarningLevelRole,
        RpmHistoryRole,
        CurrentHistoryRole,
        TemperatureHistoryRole,
        VoltageHistoryRole,
        PwmHistoryRole
    };
    Q_ENUM(Role)

    explicit MotorTelemetryModel(QObject *parent = nullptr);

    int sampleRevision() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateTelemetry(const MotorTelemetry &telemetry);
    void refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis);
    void clear();

    Q_INVOKABLE QVariant valueForMetric(int motorId, const QString &metric) const;
    Q_INVOKABLE QVariantList historyForMetric(int motorId, const QString &metric) const;
    Q_INVOKABLE int rowForMotorId(int motorId) const;
    Q_INVOKABLE bool isMotorStale(int motorId) const;
    Q_INVOKABLE int warningLevelForMotor(int motorId) const;
    Q_INVOKABLE QString statusForMotor(int motorId) const;
    Q_INVOKABLE int motorIdAt(int row) const;

signals:
    void sampleRevisionChanged();
    // Emitted for a single motor whenever its history ring buffers receive a new
    // sample. Lets chart tiles refresh only their own (motor, metric) series
    // instead of reacting to the global sampleRevision fan-out.
    void motorHistoryChanged(int motorId);

private:
    void bumpSampleRevision();
    struct MotorRow {
        MotorTelemetry telemetry;
        MotorSampleRingBuffer rpmHistory;
        MotorSampleRingBuffer currentHistory;
        MotorSampleRingBuffer temperatureHistory;
        MotorSampleRingBuffer voltageHistory;
        MotorSampleRingBuffer pwmHistory;
        bool isStale = false;
        WarningLevel warningLevel = WarningLevel::Ok;
    };

    QVector<int> rolesForSampleUpdate() const;
    QVector<int> rolesForStaleUpdate() const;

    QVector<MotorRow> m_rows;
    QHash<int, int> m_rowByMotorId;
    MotorWarningEvaluator m_warningEvaluator;
    int m_sampleRevision = 0;
};
