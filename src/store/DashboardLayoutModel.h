#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>

class DashboardLayoutModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        WidgetIdRole = Qt::UserRole + 1,
        WidgetTypeRole,
        MotorIdRole,
        MetricRole,
        ColRole,
        RowRole,
        ColSpanRole,
        RowSpanRole
    };
    Q_ENUM(Role)

    explicit DashboardLayoutModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString addWidget(const QString &type,
                                  int motorId,
                                  const QString &metric,
                                  int col,
                                  int row,
                                  int colSpan,
                                  int rowSpan);
    Q_INVOKABLE void removeWidget(const QString &widgetId);
    Q_INVOKABLE void updateWidget(const QString &widgetId,
                                  int motorId,
                                  const QString &metric,
                                  int col,
                                  int row,
                                  int colSpan,
                                  int rowSpan);
    Q_INVOKABLE void seedForMotor(int motorId);
    Q_INVOKABLE bool hasWidgetsForMotor(int motorId) const;
    Q_INVOKABLE int defaultColSpan(const QString &type) const;
    Q_INVOKABLE int defaultRowSpan(const QString &type) const;
    Q_INVOKABLE void placeWidget(const QString &widgetId, int col, int row);
    Q_INVOKABLE void resizeWidget(const QString &widgetId, int colSpan, int rowSpan);
    Q_INVOKABLE QString duplicateWidget(const QString &widgetId);
    Q_INVOKABLE bool editWidget(const QString &widgetId,
                                const QString &type,
                                int motorId,
                                const QString &metric);
    Q_INVOKABLE bool canPlace(int col, int row, int colSpan, int rowSpan) const;

private:
    struct WidgetRow {
        QString id;
        QString type;
        int motorId = 0;
        QString metric;
        int col = 1;
        int row = 1;
        int colSpan = 1;
        int rowSpan = 1;
    };

    static constexpr int kGridColumns = 16;
    static constexpr int kGridRows = 20;

    QString makeWidgetId() const;
    int indexForWidgetId(const QString &widgetId) const;
    bool overlaps(const WidgetRow &candidate, const QString &ignoreId = {}) const;
    bool isValidPlacement(int col, int row, int colSpan, int rowSpan) const;
    QVector<WidgetRow> firstFreePlacement(int colSpan, int rowSpan, const QString &ignoreId = {}) const;
    void loadFromSettings();
    void persistToSettings();
    void migrateLegacyPositions();

    QVector<WidgetRow> m_widgets;
    QHash<QString, int> m_rowByWidgetId;
};
