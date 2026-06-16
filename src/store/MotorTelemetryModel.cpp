#include "MotorTelemetryModel.h"

#include <QVariantList>

namespace {

QVariantList historyToVariantList(const MotorSampleRingBuffer &buffer)
{
    const QVector<double> values = buffer.orderedValues();
    QVariantList list;
    list.reserve(values.size());
    for (double value : values) {
        list.push_back(value);
    }
    return list;
}

} // namespace

MotorTelemetryModel::MotorTelemetryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MotorTelemetryModel::sampleRevision() const
{
    return m_sampleRevision;
}

void MotorTelemetryModel::bumpSampleRevision()
{
    ++m_sampleRevision;
    emit sampleRevisionChanged();
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
    case RpmHistoryRole:
        return historyToVariantList(row.rpmHistory);
    case CurrentHistoryRole:
        return historyToVariantList(row.currentHistory);
    case TemperatureHistoryRole:
        return historyToVariantList(row.temperatureHistory);
    case VoltageHistoryRole:
        return historyToVariantList(row.voltageHistory);
    case PwmHistoryRole:
        return historyToVariantList(row.pwmHistory);
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
        {WarningLevelRole, "warningLevel"},
        {RpmHistoryRole, "rpmHistory"},
        {CurrentHistoryRole, "currentHistory"},
        {TemperatureHistoryRole, "temperatureHistory"},
        {VoltageHistoryRole, "voltageHistory"},
        {PwmHistoryRole, "pwmHistory"}
    };
}

QVariant MotorTelemetryModel::valueForMetric(int motorId, const QString &metric) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return metric == QLatin1String("status") ? QVariant(QString()) : QVariant(0.0);
    }

    const MotorTelemetry &telemetry = m_rows.at(*existing).telemetry;
    if (metric == QLatin1String("rpm")) {
        return telemetry.rpm;
    }
    if (metric == QLatin1String("voltage")) {
        return telemetry.voltage;
    }
    if (metric == QLatin1String("current")) {
        return telemetry.current;
    }
    if (metric == QLatin1String("temperatureCelsius")) {
        return telemetry.temperatureCelsius;
    }
    if (metric == QLatin1String("pwm")) {
        return telemetry.pwm;
    }
    if (metric == QLatin1String("status")) {
        return telemetry.status;
    }

    return metric == QLatin1String("status") ? QVariant(QString()) : QVariant(0.0);
}

QVariantList MotorTelemetryModel::historyForMetric(int motorId, const QString &metric) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return {};
    }

    const MotorRow &row = m_rows.at(*existing);
    if (metric == QLatin1String("rpm")) {
        return historyToVariantList(row.rpmHistory);
    }
    if (metric == QLatin1String("voltage")) {
        return historyToVariantList(row.voltageHistory);
    }
    if (metric == QLatin1String("current")) {
        return historyToVariantList(row.currentHistory);
    }
    if (metric == QLatin1String("temperatureCelsius")) {
        return historyToVariantList(row.temperatureHistory);
    }
    if (metric == QLatin1String("pwm")) {
        return historyToVariantList(row.pwmHistory);
    }

    return {};
}

int MotorTelemetryModel::rowForMotorId(int motorId) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return -1;
    }
    return *existing;
}

bool MotorTelemetryModel::isMotorStale(int motorId) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return true;
    }
    return m_rows.at(*existing).isStale;
}

int MotorTelemetryModel::warningLevelForMotor(int motorId) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return static_cast<int>(WarningLevel::Stale);
    }
    return static_cast<int>(m_rows.at(*existing).warningLevel);
}

QString MotorTelemetryModel::statusForMotor(int motorId) const
{
    const auto existing = m_rowByMotorId.constFind(motorId);
    if (existing == m_rowByMotorId.constEnd()) {
        return {};
    }
    return m_rows.at(*existing).telemetry.status;
}

int MotorTelemetryModel::motorIdAt(int row) const
{
    if (row < 0 || row >= m_rows.size()) {
        return 0;
    }
    return m_rows.at(row).telemetry.motorId;
}

void MotorTelemetryModel::updateTelemetry(const MotorTelemetry &telemetry)
{
    const auto existing = m_rowByMotorId.constFind(telemetry.motorId);

    if (existing == m_rowByMotorId.constEnd()) {
        const int rowIndex = m_rows.size();
        beginInsertRows(QModelIndex(), rowIndex, rowIndex);
        m_rowByMotorId.insert(telemetry.motorId, rowIndex);
        MotorRow newRow;
        newRow.telemetry = telemetry;
        newRow.warningLevel = m_warningEvaluator.evaluate(telemetry, false);
        newRow.rpmHistory.push(telemetry.rpm);
        newRow.currentHistory.push(telemetry.current);
        newRow.temperatureHistory.push(telemetry.temperatureCelsius);
        newRow.voltageHistory.push(telemetry.voltage);
        newRow.pwmHistory.push(telemetry.pwm);
        m_rows.push_back(newRow);
        endInsertRows();
        emit motorHistoryChanged(telemetry.motorId);
        bumpSampleRevision();
        return;
    }

    MotorRow &row = m_rows[*existing];
    row.telemetry = telemetry;
    row.isStale = false;
    row.warningLevel = m_warningEvaluator.evaluate(telemetry, false);
    row.rpmHistory.push(telemetry.rpm);
    row.currentHistory.push(telemetry.current);
    row.temperatureHistory.push(telemetry.temperatureCelsius);
    row.voltageHistory.push(telemetry.voltage);
    row.pwmHistory.push(telemetry.pwm);

    const QModelIndex changedIndex = index(*existing);
    emit dataChanged(changedIndex, changedIndex, rolesForSampleUpdate());
    emit motorHistoryChanged(telemetry.motorId);
    bumpSampleRevision();
}

void MotorTelemetryModel::refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis)
{
    bool changed = false;
    for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        MotorRow &row = m_rows[rowIndex];
        const bool isStale = nowMillis - row.telemetry.timestampMillis > staleThresholdMillis;
        const WarningLevel warningLevel = m_warningEvaluator.evaluate(row.telemetry, isStale);

        if (row.isStale == isStale && row.warningLevel == warningLevel) {
            continue;
        }

        row.isStale = isStale;
        row.warningLevel = warningLevel;
        changed = true;

        const QModelIndex changedIndex = index(rowIndex);
        emit dataChanged(changedIndex, changedIndex, rolesForStaleUpdate());
    }

    if (changed) {
        bumpSampleRevision();
    }
}

void MotorTelemetryModel::clear()
{
    if (m_rows.isEmpty()) {
        return;
    }

    beginResetModel();
    m_rows.clear();
    m_rowByMotorId.clear();
    endResetModel();
    bumpSampleRevision();
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
        WarningLevelRole,
        RpmHistoryRole,
        CurrentHistoryRole,
        TemperatureHistoryRole,
        VoltageHistoryRole,
        PwmHistoryRole
    };
}

QVector<int> MotorTelemetryModel::rolesForStaleUpdate() const
{
    return {
        IsStaleRole,
        WarningLevelRole
    };
}
