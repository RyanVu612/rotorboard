#include "BatteryTelemetryModel.h"

#include <QVariantList>

namespace {

QVariantList cellVoltagesToVariantList(const QVector<double> &cellVoltages)
{
    QVariantList list;
    list.reserve(cellVoltages.size());
    for (double voltage : cellVoltages) {
        list.push_back(voltage);
    }
    return list;
}

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

BatteryTelemetryModel::BatteryTelemetryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int BatteryTelemetryModel::sampleRevision() const
{
    return m_sampleRevision;
}

void BatteryTelemetryModel::bumpSampleRevision()
{
    ++m_sampleRevision;
    emit sampleRevisionChanged();
}

int BatteryTelemetryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.size();
}

QVariant BatteryTelemetryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }

    const BatteryRow &row = m_rows.at(index.row());
    const BatteryTelemetry &telemetry = row.telemetry;

    switch (role) {
    case BatteryIdRole:
        return telemetry.batteryId;
    case VoltageRole:
        return telemetry.voltage;
    case CurrentRole:
        return telemetry.current;
    case BatteryRemainingRole:
        return telemetry.batteryRemaining;
    case TemperatureCelsiusRole:
        return telemetry.temperatureCelsius;
    case CurrentConsumedMahRole:
        return telemetry.currentConsumedMah;
    case CellVoltagesRole:
        return cellVoltagesToVariantList(telemetry.cellVoltages);
    case TimestampMillisRole:
        return telemetry.timestampMillis;
    case IsStaleRole:
        return row.isStale;
    case WarningLevelRole:
        return static_cast<int>(row.warningLevel);
    case VoltageHistoryRole:
        return historyToVariantList(row.voltageHistory);
    case CurrentHistoryRole:
        return historyToVariantList(row.currentHistory);
    case BatteryRemainingHistoryRole:
        return historyToVariantList(row.batteryRemainingHistory);
    case TemperatureHistoryRole:
        return historyToVariantList(row.temperatureHistory);
    case CurrentConsumedMahHistoryRole:
        return historyToVariantList(row.currentConsumedMahHistory);
    default:
        return {};
    }
}

QHash<int, QByteArray> BatteryTelemetryModel::roleNames() const
{
    return {
        {BatteryIdRole, "batteryId"},
        {VoltageRole, "voltage"},
        {CurrentRole, "current"},
        {BatteryRemainingRole, "batteryRemaining"},
        {TemperatureCelsiusRole, "temperatureCelsius"},
        {CurrentConsumedMahRole, "currentConsumedMah"},
        {CellVoltagesRole, "cellVoltages"},
        {TimestampMillisRole, "timestampMillis"},
        {IsStaleRole, "isStale"},
        {WarningLevelRole, "warningLevel"},
        {VoltageHistoryRole, "voltageHistory"},
        {CurrentHistoryRole, "currentHistory"},
        {BatteryRemainingHistoryRole, "batteryRemainingHistory"},
        {TemperatureHistoryRole, "temperatureHistory"},
        {CurrentConsumedMahHistoryRole, "currentConsumedMahHistory"}
    };
}

QVariant BatteryTelemetryModel::valueForMetric(int batteryId, const QString &metric) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return 0.0;
    }

    const BatteryTelemetry &telemetry = m_rows.at(*existing).telemetry;
    if (metric == QLatin1String("voltage")) {
        return telemetry.voltage;
    }
    if (metric == QLatin1String("current")) {
        return telemetry.current;
    }
    if (metric == QLatin1String("batteryRemaining")) {
        return telemetry.batteryRemaining;
    }
    if (metric == QLatin1String("temperatureCelsius")) {
        return telemetry.temperatureCelsius;
    }
    if (metric == QLatin1String("currentConsumedMah")) {
        return telemetry.currentConsumedMah;
    }

    return 0.0;
}

QVariantList BatteryTelemetryModel::historyForMetric(int batteryId, const QString &metric) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return {};
    }

    const BatteryRow &row = m_rows.at(*existing);
    if (metric == QLatin1String("voltage")) {
        return historyToVariantList(row.voltageHistory);
    }
    if (metric == QLatin1String("current")) {
        return historyToVariantList(row.currentHistory);
    }
    if (metric == QLatin1String("batteryRemaining")) {
        return historyToVariantList(row.batteryRemainingHistory);
    }
    if (metric == QLatin1String("temperatureCelsius")) {
        return historyToVariantList(row.temperatureHistory);
    }
    if (metric == QLatin1String("currentConsumedMah")) {
        return historyToVariantList(row.currentConsumedMahHistory);
    }

    return {};
}

QVariantList BatteryTelemetryModel::cellVoltagesForBattery(int batteryId) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return {};
    }
    return cellVoltagesToVariantList(m_rows.at(*existing).telemetry.cellVoltages);
}

int BatteryTelemetryModel::rowForBatteryId(int batteryId) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return -1;
    }
    return *existing;
}

bool BatteryTelemetryModel::isBatteryStale(int batteryId) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return true;
    }
    return m_rows.at(*existing).isStale;
}

int BatteryTelemetryModel::warningLevelForBattery(int batteryId) const
{
    const auto existing = m_rowByBatteryId.constFind(batteryId);
    if (existing == m_rowByBatteryId.constEnd()) {
        return static_cast<int>(WarningLevel::Stale);
    }
    return static_cast<int>(m_rows.at(*existing).warningLevel);
}

int BatteryTelemetryModel::batteryIdAt(int row) const
{
    if (row < 0 || row >= m_rows.size()) {
        return 0;
    }
    return m_rows.at(row).telemetry.batteryId;
}

void BatteryTelemetryModel::updateTelemetry(const BatteryTelemetry &telemetry)
{
    const auto existing = m_rowByBatteryId.constFind(telemetry.batteryId);

    if (existing == m_rowByBatteryId.constEnd()) {
        const int rowIndex = m_rows.size();
        beginInsertRows(QModelIndex(), rowIndex, rowIndex);
        m_rowByBatteryId.insert(telemetry.batteryId, rowIndex);
        BatteryRow newRow;
        newRow.telemetry = telemetry;
        newRow.warningLevel = m_warningEvaluator.evaluate(telemetry, false);
        newRow.voltageHistory.push(telemetry.voltage);
        newRow.currentHistory.push(telemetry.current);
        newRow.batteryRemainingHistory.push(telemetry.batteryRemaining);
        newRow.temperatureHistory.push(telemetry.temperatureCelsius);
        newRow.currentConsumedMahHistory.push(telemetry.currentConsumedMah);
        m_rows.push_back(newRow);
        endInsertRows();
        bumpSampleRevision();
        return;
    }

    BatteryRow &row = m_rows[*existing];
    row.telemetry = telemetry;
    row.isStale = false;
    row.warningLevel = m_warningEvaluator.evaluate(telemetry, false);
    row.voltageHistory.push(telemetry.voltage);
    row.currentHistory.push(telemetry.current);
    row.batteryRemainingHistory.push(telemetry.batteryRemaining);
    row.temperatureHistory.push(telemetry.temperatureCelsius);
    row.currentConsumedMahHistory.push(telemetry.currentConsumedMah);

    const QModelIndex changedIndex = index(*existing);
    emit dataChanged(changedIndex, changedIndex, rolesForSampleUpdate());
    bumpSampleRevision();
}

void BatteryTelemetryModel::refreshStaleState(qint64 nowMillis, qint64 staleThresholdMillis)
{
    bool changed = false;
    for (int rowIndex = 0; rowIndex < m_rows.size(); ++rowIndex) {
        BatteryRow &row = m_rows[rowIndex];
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

void BatteryTelemetryModel::clear()
{
    if (m_rows.isEmpty()) {
        return;
    }

    beginResetModel();
    m_rows.clear();
    m_rowByBatteryId.clear();
    endResetModel();
    bumpSampleRevision();
}

QVector<int> BatteryTelemetryModel::rolesForSampleUpdate() const
{
    return {
        BatteryIdRole,
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
}

QVector<int> BatteryTelemetryModel::rolesForStaleUpdate() const
{
    return {
        IsStaleRole,
        WarningLevelRole
    };
}
