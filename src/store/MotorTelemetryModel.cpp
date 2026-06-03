#include "MotorTelemetryModel.h"

MotorTelemetryModel::MotorTelemetryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MotorTelemetryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_rows.size();
}

QVariant MotorTelemetryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }

    const MotorRow &row = m_rows.at(index.row());
    const MotorTelemetry &telemetry = row.telemetry;

    switch (role) {
    case MotorIdRole:
        return telemetry.motorId;
    case RpmRole:
        return telemetry.rpm;
    case VoltageRole:
        return telemetry.voltage;
    case CurrentRole:
        return telemetry.current;
    case TemperatureCelsiusRole:
        return telemetry.temperatureCelsius;
    case PwmRole:
        return telemetry.pwm;
    case StatusRole:
        return telemetry.status;
    case TimestampMillisRole:
        return telemetry.timestampMillis;
    case IsStaleRole:
        return row.isStale;
    case WarningLevelRole:
        return static_cast<int>(row.warningLevel);
    default:
        return {};
    }
}

QHash<int, QByteArray> MotorTelemetryModel::roleNames() const
{
    return {
        {MotorIdRole, "motorId"},
        {RpmRole, "rpm"},
        {VoltageRole, "voltage"},
        {CurrentRole, "current"},
        {TemperatureCelsiusRole, "temperatureCelsius"},
        {PwmRole, "pwm"},
        {StatusRole, "status"},
        {TimestampMillisRole, "timestampMillis"},
        {IsStaleRole, "isStale"},
        {WarningLevelRole, "warningLevel"}
    };
}

void MotorTelemetryModel::updateTelemetry(const MotorTelemetry &telemetry)
{
    const auto existing = m_rowByMotorId.constFind(telemetry.motorId);

    if (existing == m_rowByMotorId.constEnd()) {
        const int row = m_rows.size();
        beginInsertRows(QModelIndex(), row, row);
        m_rowByMotorId.insert(telemetry.motorId, row);
        m_rows.push_back({telemetry, false, m_warningEvaluator.evaluate(telemetry, false)});
        endInsertRows();
        return;
    }

    MotorRow &row = m_rows[*existing];
    row.telemetry = telemetry;
    row.isStale = false;
    row.warningLevel = m_warningEvaluator.evaluate(telemetry, false);

    const QModelIndex changedIndex = index(*existing);
    emit dataChanged(changedIndex, changedIndex, rolesForSampleUpdate());
}

void MotorTelemetryModel::refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis)
{
    for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        MotorRow &row = m_rows[rowIndex];
        const bool isStale = nowMillis - row.telemetry.timestampMillis > staleThresholdMillis;
        const WarningLevel warningLevel = m_warningEvaluator.evaluate(row.telemetry, isStale);

        if (row.isStale == isStale && row.warningLevel == warningLevel) {
            continue;
        }

        row.isStale = isStale;
        row.warningLevel = warningLevel;

        const QModelIndex changedIndex = index(rowIndex);
        emit dataChanged(changedIndex, changedIndex, rolesForStaleUpdate());
    }
}

QVector<int> MotorTelemetryModel::rolesForSampleUpdate() const
{
    return {
        MotorIdRole,
        RpmRole,
        VoltageRole,
        CurrentRole,
        TemperatureCelsiusRole,
        PwmRole,
        StatusRole,
        TimestampMillisRole,
        IsStaleRole,
        WarningLevelRole
    };
}

QVector<int> MotorTelemetryModel::rolesForStaleUpdate() const
{
    return {
        IsStaleRole,
        WarningLevelRole
    };
}
