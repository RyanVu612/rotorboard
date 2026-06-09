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
        TemperatureHistoryRole
    };
    Q_ENUM(Role)

    explicit MotorTelemetryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateTelemetry(const MotorTelemetry &telemetry);
    void refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis);

private:
    struct MotorRow {
        MotorTelemetry telemetry;
        MotorSampleRingBuffer rpmHistory;
        MotorSampleRingBuffer currentHistory;
        MotorSampleRingBuffer temperatureHistory;
        bool isStale = false;
        WarningLevel warningLevel = WarningLevel::Ok;
    };

    QVector<int> rolesForSampleUpdate() const;
    QVector<int> rolesForStaleUpdate() const;

    QVector<MotorRow> m_rows;
    QHash<int, int> m_rowByMotorId;
    MotorWarningEvaluator m_warningEvaluator;
};
