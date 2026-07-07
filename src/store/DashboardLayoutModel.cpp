#include "DashboardLayoutModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QUuid>

namespace {

bool rectanglesOverlap(int colA, int rowA, int colSpanA, int rowSpanA,
                       int colB, int rowB, int colSpanB, int rowSpanB)
{
    return colA < colB + colSpanB && colA + colSpanA > colB &&
           rowA < rowB + rowSpanB && rowA + rowSpanA > rowB;
}

} // namespace

DashboardLayoutModel::DashboardLayoutModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadFromSettings();
}

int DashboardLayoutModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_widgets.size();
}

QVariant DashboardLayoutModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_widgets.size()) {
        return {};
    }

    const WidgetRow &widget = m_widgets.at(index.row());
    switch (role) {
    case WidgetIdRole:
        return widget.id;
    case WidgetTypeRole:
        return widget.type;
    case MotorIdRole:
        return widget.motorId;
    case MetricRole:
        return widget.metric;
    case ColRole:
        return widget.col;
    case RowRole:
        return widget.row;
    case ColSpanRole:
        return widget.colSpan;
    case RowSpanRole:
        return widget.rowSpan;
    default:
        return {};
    }
}

QHash<int, QByteArray> DashboardLayoutModel::roleNames() const
{
    return {
        {WidgetIdRole, "widgetId"},
        {WidgetTypeRole, "widgetType"},
        {MotorIdRole, "motorId"},
        {MetricRole, "metric"},
        {ColRole, "col"},
        {RowRole, "row"},
        {ColSpanRole, "colSpan"},
        {RowSpanRole, "rowSpan"}
    };
}

QString DashboardLayoutModel::addWidget(const QString &type,
                                        int motorId,
                                        const QString &metric,
                                        int col,
                                        int row,
                                        int colSpan,
                                        int rowSpan)
{
    WidgetRow candidate;
    candidate.id = makeWidgetId();
    candidate.type = type;
    candidate.motorId = motorId;
    candidate.metric = metric;
    candidate.col = col;
    candidate.row = row;
    candidate.colSpan = colSpan > 0 ? colSpan : defaultColSpan(type);
    candidate.rowSpan = rowSpan > 0 ? rowSpan : defaultRowSpan(type);

    if (!isValidPlacement(candidate.col, candidate.row, candidate.colSpan, candidate.rowSpan) ||
        overlaps(candidate)) {
        const QVector<WidgetRow> freeSpot = firstFreePlacement(candidate.colSpan, candidate.rowSpan);
        if (freeSpot.isEmpty()) {
            return {};
        }
        candidate.col = freeSpot.first().col;
        candidate.row = freeSpot.first().row;
    }

    const int rowIndex = m_widgets.size();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    m_widgets.push_back(candidate);
    m_rowByWidgetId.insert(candidate.id, rowIndex);
    endInsertRows();
    persistToSettings();
    return candidate.id;
}

void DashboardLayoutModel::removeWidget(const QString &widgetId)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0) {
        return;
    }

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    m_widgets.removeAt(rowIndex);
    endRemoveRows();

    m_rowByWidgetId.clear();
    for (int i = 0; i < m_widgets.size(); ++i) {
        m_rowByWidgetId.insert(m_widgets.at(i).id, i);
    }
    persistToSettings();
}

void DashboardLayoutModel::updateWidget(const QString &widgetId,
                                        int motorId,
                                        const QString &metric,
                                        int col,
                                        int row,
                                        int colSpan,
                                        int rowSpan)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0) {
        return;
    }

    WidgetRow candidate = m_widgets.at(rowIndex);
    candidate.motorId = motorId;
    candidate.metric = metric;
    candidate.col = col;
    candidate.row = row;
    candidate.colSpan = colSpan;
    candidate.rowSpan = rowSpan;

    if (!isValidPlacement(candidate.col, candidate.row, candidate.colSpan, candidate.rowSpan) ||
        overlaps(candidate, widgetId)) {
        return;
    }

    m_widgets[rowIndex] = candidate;
    const QModelIndex changedIndex = index(rowIndex);
    emit dataChanged(changedIndex, changedIndex);
    persistToSettings();
}

void DashboardLayoutModel::seedForMotor(int motorId)
{
    if (hasWidgetsForMotor(motorId)) {
        return;
    }

    addWidget(QStringLiteral("chart"), motorId, QStringLiteral("rpm"), 1, 1, 4, 3);

    const QVector<WidgetRow> currentSpot = firstFreePlacement(1, 1);
    if (!currentSpot.isEmpty()) {
        addWidget(QStringLiteral("value"),
                  motorId,
                  QStringLiteral("current"),
                  currentSpot.first().col,
                  currentSpot.first().row,
                  1,
                  1);
    }

    const QVector<WidgetRow> tempSpot = firstFreePlacement(1, 1);
    if (!tempSpot.isEmpty()) {
        addWidget(QStringLiteral("value"),
                  motorId,
                  QStringLiteral("temperatureCelsius"),
                  tempSpot.first().col,
                  tempSpot.first().row,
                  1,
                  1);
    }
}

bool DashboardLayoutModel::hasWidgetsForMotor(int motorId) const
{
    for (const WidgetRow &widget : m_widgets) {
        if (widget.motorId == motorId) {
            return true;
        }
    }
    return false;
}

int DashboardLayoutModel::defaultColSpan(const QString &type) const
{
    if (type == QLatin1String("chart")) {
        return 4;
    }
    if (type == QLatin1String("motorSummary") || type == QLatin1String("batterySummary")) {
        return 3;
    }
    return 1;
}

int DashboardLayoutModel::defaultRowSpan(const QString &type) const
{
    if (type == QLatin1String("chart")) {
        return 3;
    }
    if (type == QLatin1String("motorSummary") || type == QLatin1String("batterySummary")) {
        return 4;
    }
    return 1;
}

void DashboardLayoutModel::placeWidget(const QString &widgetId, int col, int row)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0) {
        return;
    }

    const WidgetRow current = m_widgets.at(rowIndex);
    if (current.col == col && current.row == row) {
        return;
    }

    WidgetRow moving = current;
    moving.col = col;
    moving.row = row;

    if (!isValidPlacement(moving.col, moving.row, moving.colSpan, moving.rowSpan)) {
        return;
    }

    QString occupantId;
    for (const WidgetRow &widget : m_widgets) {
        if (widget.id == widgetId) {
            continue;
        }
        if (widget.col == col && widget.row == row) {
            occupantId = widget.id;
            break;
        }
    }

    if (!occupantId.isEmpty()) {
        const int occupantIndex = indexForWidgetId(occupantId);
        if (occupantIndex < 0) {
            return;
        }

        WidgetRow occupant = m_widgets.at(occupantIndex);
        occupant.col = current.col;
        occupant.row = current.row;

        m_widgets[rowIndex] = moving;
        m_widgets[occupantIndex] = occupant;

        emit dataChanged(index(rowIndex), index(rowIndex));
        emit dataChanged(index(occupantIndex), index(occupantIndex));
        persistToSettings();
        return;
    }

    if (overlaps(moving, widgetId)) {
        return;
    }

    updateWidget(widgetId,
                 moving.motorId,
                 moving.metric,
                 moving.col,
                 moving.row,
                 moving.colSpan,
                 moving.rowSpan);
}

void DashboardLayoutModel::resizeWidget(const QString &widgetId, int colSpan, int rowSpan)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0 || colSpan < 1 || rowSpan < 1) {
        return;
    }

    WidgetRow candidate = m_widgets.at(rowIndex);
    candidate.colSpan = colSpan;
    candidate.rowSpan = rowSpan;

    if (!isValidPlacement(candidate.col, candidate.row, candidate.colSpan, candidate.rowSpan) ||
        overlaps(candidate, widgetId)) {
        return;
    }

    m_widgets[rowIndex] = candidate;
    emit dataChanged(index(rowIndex), index(rowIndex));
    persistToSettings();
}

QString DashboardLayoutModel::duplicateWidget(const QString &widgetId)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0) {
        return {};
    }

    const WidgetRow &source = m_widgets.at(rowIndex);
    const QVector<WidgetRow> freeSpot = firstFreePlacement(source.colSpan, source.rowSpan);
    if (freeSpot.isEmpty()) {
        return {};
    }

    return addWidget(source.type,
                     source.motorId,
                     source.metric,
                     freeSpot.first().col,
                     freeSpot.first().row,
                     source.colSpan,
                     source.rowSpan);
}

bool DashboardLayoutModel::editWidget(const QString &widgetId,
                                      const QString &type,
                                      int motorId,
                                      const QString &metric)
{
    const int rowIndex = indexForWidgetId(widgetId);
    if (rowIndex < 0) {
        return false;
    }

    WidgetRow candidate = m_widgets.at(rowIndex);
    const bool typeChanged = candidate.type != type;
    candidate.type = type;
    candidate.motorId = motorId;
    candidate.metric = metric;

    if (typeChanged) {
        candidate.colSpan = defaultColSpan(type);
        candidate.rowSpan = defaultRowSpan(type);
        if (!isValidPlacement(candidate.col, candidate.row, candidate.colSpan, candidate.rowSpan) ||
            overlaps(candidate, widgetId)) {
            const QVector<WidgetRow> freeSpot =
                firstFreePlacement(candidate.colSpan, candidate.rowSpan, widgetId);
            if (freeSpot.isEmpty()) {
                return false;
            }
            candidate.col = freeSpot.first().col;
            candidate.row = freeSpot.first().row;
        }
    }

    m_widgets[rowIndex] = candidate;
    const QModelIndex changedIndex = index(rowIndex);
    emit dataChanged(changedIndex, changedIndex);
    persistToSettings();
    return true;
}

bool DashboardLayoutModel::canPlace(int col, int row, int colSpan, int rowSpan) const
{
    WidgetRow candidate;
    candidate.col = col;
    candidate.row = row;
    candidate.colSpan = colSpan;
    candidate.rowSpan = rowSpan;
    return isValidPlacement(col, row, colSpan, rowSpan) && !overlaps(candidate);
}

QString DashboardLayoutModel::makeWidgetId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int DashboardLayoutModel::indexForWidgetId(const QString &widgetId) const
{
    const auto found = m_rowByWidgetId.constFind(widgetId);
    if (found == m_rowByWidgetId.constEnd()) {
        return -1;
    }
    return *found;
}

bool DashboardLayoutModel::overlaps(const WidgetRow &candidate, const QString &ignoreId) const
{
    for (const WidgetRow &widget : m_widgets) {
        if (widget.id == ignoreId) {
            continue;
        }
        if (rectanglesOverlap(candidate.col,
                              candidate.row,
                              candidate.colSpan,
                              candidate.rowSpan,
                              widget.col,
                              widget.row,
                              widget.colSpan,
                              widget.rowSpan)) {
            return true;
        }
    }
    return false;
}

bool DashboardLayoutModel::isValidPlacement(int col, int row, int colSpan, int rowSpan) const
{
    return col >= 1 && row >= 1 && colSpan >= 1 && rowSpan >= 1 &&
           col + colSpan - 1 <= kGridColumns && row + rowSpan - 1 <= kGridRows;
}

QVector<DashboardLayoutModel::WidgetRow> DashboardLayoutModel::firstFreePlacement(int colSpan,
                                                                                  int rowSpan,
                                                                                  const QString &ignoreId) const
{
    for (int row = 1; row <= kGridRows; ++row) {
        for (int col = 1; col <= kGridColumns; ++col) {
            WidgetRow candidate;
            candidate.col = col;
            candidate.row = row;
            candidate.colSpan = colSpan;
            candidate.rowSpan = rowSpan;
            if (isValidPlacement(candidate.col, candidate.row, candidate.colSpan, candidate.rowSpan) &&
                !overlaps(candidate, ignoreId)) {
                return {candidate};
            }
        }
    }
    return {};
}

void DashboardLayoutModel::loadFromSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Dashboard"));
    const QString widgetsJson = settings.value(QStringLiteral("widgetsJson")).toString();
    settings.endGroup();

    if (!widgetsJson.isEmpty()) {
        const QJsonDocument document = QJsonDocument::fromJson(widgetsJson.toUtf8());
        if (document.isArray()) {
            beginResetModel();
            m_widgets.clear();
            m_rowByWidgetId.clear();
            for (const QJsonValue &value : document.array()) {
                if (!value.isObject()) {
                    continue;
                }
                const QJsonObject object = value.toObject();
                WidgetRow widget;
                widget.id = object.value(QStringLiteral("id")).toString();
                widget.type = object.value(QStringLiteral("type")).toString();
                widget.motorId = object.value(QStringLiteral("motorId")).toInt();
                widget.metric = object.value(QStringLiteral("metric")).toString();
                widget.col = object.value(QStringLiteral("col")).toInt(1);
                widget.row = object.value(QStringLiteral("row")).toInt(1);
                widget.colSpan = object.value(QStringLiteral("colSpan")).toInt(1);
                widget.rowSpan = object.value(QStringLiteral("rowSpan")).toInt(1);
                if (widget.id.isEmpty() || widget.type.isEmpty()) {
                    continue;
                }
                m_rowByWidgetId.insert(widget.id, m_widgets.size());
                m_widgets.push_back(widget);
            }
            endResetModel();
            return;
        }
    }

    migrateLegacyPositions();
}

void DashboardLayoutModel::persistToSettings()
{
    QJsonArray array;
    for (const WidgetRow &widget : m_widgets) {
        QJsonObject object;
        object.insert(QStringLiteral("id"), widget.id);
        object.insert(QStringLiteral("type"), widget.type);
        object.insert(QStringLiteral("motorId"), widget.motorId);
        object.insert(QStringLiteral("metric"), widget.metric);
        object.insert(QStringLiteral("col"), widget.col);
        object.insert(QStringLiteral("row"), widget.row);
        object.insert(QStringLiteral("colSpan"), widget.colSpan);
        object.insert(QStringLiteral("rowSpan"), widget.rowSpan);
        array.push_back(object);
    }

    QSettings settings;
    settings.beginGroup(QStringLiteral("Dashboard"));
    settings.setValue(QStringLiteral("widgetsJson"), QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact)));
    settings.endGroup();
}

void DashboardLayoutModel::migrateLegacyPositions()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("MotorGrid"));
    const QString positionsJson = settings.value(QStringLiteral("positionsJson")).toString();
    settings.endGroup();

    if (positionsJson.isEmpty() || positionsJson == QLatin1String("{}")) {
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(positionsJson.toUtf8());
    if (!document.isObject()) {
        return;
    }

    const QJsonObject object = document.object();
    beginResetModel();
    m_widgets.clear();
    m_rowByWidgetId.clear();

    for (auto it = object.begin(); it != object.end(); ++it) {
        if (!it.value().isObject()) {
            continue;
        }
        const QJsonObject position = it.value().toObject();
        WidgetRow widget;
        widget.id = makeWidgetId();
        widget.type = QStringLiteral("motorSummary");
        widget.motorId = it.key().toInt();
        widget.metric = QString();
        widget.col = position.value(QStringLiteral("col")).toInt(1);
        widget.row = position.value(QStringLiteral("row")).toInt(1);
        widget.colSpan = defaultColSpan(widget.type);
        widget.rowSpan = defaultRowSpan(widget.type);
        if (!isValidPlacement(widget.col, widget.row, widget.colSpan, widget.rowSpan)) {
            continue;
        }
        m_rowByWidgetId.insert(widget.id, m_widgets.size());
        m_widgets.push_back(widget);
    }

    endResetModel();
    if (!m_widgets.isEmpty()) {
        persistToSettings();
    }
}
