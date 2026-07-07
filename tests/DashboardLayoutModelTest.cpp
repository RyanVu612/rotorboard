#include "store/DashboardLayoutModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QtTest>

class DashboardLayoutModelTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void duplicatePlacesCopyAtFirstFreeSpot();
    void duplicateWithNoRoomAddsNothing();
    void duplicateUnknownIdAddsNothing();
    void editWidgetUpdatesMotorAndMetricKeepingSpan();
    void editWidgetTypeSwitchAppliesDefaultSpan();
    void editWidgetTypeSwitchRelocatesWhenBlocked();
    void editWidgetTypeSwitchWithNoRoomIsRejected();
    void batterySummaryUsesMotorSummaryDefaults();
    void loadsExistingMotorOnlyLayoutUnchanged();
    void canPlaceReportsOverlapAndBounds();

private:
    int widgetRow(DashboardLayoutModel &model, const QString &widgetId, int role) const;
};

void DashboardLayoutModelTest::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("rotorboard-test"));
    QCoreApplication::setApplicationName(QStringLiteral("layout-model-test"));
}

void DashboardLayoutModelTest::init()
{
    QSettings settings;
    settings.clear();
    settings.sync();
}

int DashboardLayoutModelTest::widgetRow(DashboardLayoutModel &model,
                                        const QString &widgetId,
                                        int role) const
{
    for (int i = 0; i < model.rowCount(); ++i) {
        const QModelIndex index = model.index(i);
        if (model.data(index, DashboardLayoutModel::WidgetIdRole).toString() == widgetId) {
            return model.data(index, role).toInt();
        }
    }
    return -1;
}

void DashboardLayoutModelTest::duplicatePlacesCopyAtFirstFreeSpot()
{
    DashboardLayoutModel model;
    const QString sourceId =
        model.addWidget(QStringLiteral("chart"), 1, QStringLiteral("rpm"), 1, 1, 4, 3);
    QVERIFY(!sourceId.isEmpty());

    const QString copyId = model.duplicateWidget(sourceId);
    QVERIFY(!copyId.isEmpty());
    QCOMPARE(model.rowCount(), 2);

    // First free spot for a 4x3 card with a 4x3 card at (1,1) is (5,1).
    QCOMPARE(widgetRow(model, copyId, DashboardLayoutModel::ColRole), 5);
    QCOMPARE(widgetRow(model, copyId, DashboardLayoutModel::RowRole), 1);
    QCOMPARE(widgetRow(model, copyId, DashboardLayoutModel::ColSpanRole), 4);
    QCOMPARE(widgetRow(model, copyId, DashboardLayoutModel::RowSpanRole), 3);
}

void DashboardLayoutModelTest::duplicateWithNoRoomAddsNothing()
{
    DashboardLayoutModel model;
    // One widget filling the whole 16x20 grid leaves no room for a copy.
    const QString sourceId =
        model.addWidget(QStringLiteral("chart"), 1, QStringLiteral("rpm"), 1, 1, 16, 20);
    QVERIFY(!sourceId.isEmpty());

    QCOMPARE(model.duplicateWidget(sourceId), QString());
    QCOMPARE(model.rowCount(), 1);
}

void DashboardLayoutModelTest::duplicateUnknownIdAddsNothing()
{
    DashboardLayoutModel model;
    QCOMPARE(model.duplicateWidget(QStringLiteral("missing")), QString());
    QCOMPARE(model.rowCount(), 0);
}

void DashboardLayoutModelTest::editWidgetUpdatesMotorAndMetricKeepingSpan()
{
    DashboardLayoutModel model;
    const QString id =
        model.addWidget(QStringLiteral("chart"), 1, QStringLiteral("rpm"), 2, 2, 5, 4);

    QVERIFY(model.editWidget(id, QStringLiteral("chart"), 7, QStringLiteral("voltage")));

    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::MotorIdRole), 7);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColRole), 2);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowRole), 2);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColSpanRole), 5);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowSpanRole), 4);

    const QModelIndex index = model.index(0);
    QCOMPARE(model.data(index, DashboardLayoutModel::MetricRole).toString(),
             QStringLiteral("voltage"));
}

void DashboardLayoutModelTest::editWidgetTypeSwitchAppliesDefaultSpan()
{
    DashboardLayoutModel model;
    const QString id =
        model.addWidget(QStringLiteral("value"), 1, QStringLiteral("rpm"), 1, 1, 1, 1);

    QVERIFY(model.editWidget(id, QStringLiteral("chart"), 1, QStringLiteral("rpm")));

    // Chart default span is 4x3 and it fits at (1,1) on an empty grid.
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColSpanRole), 4);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowSpanRole), 3);
}

void DashboardLayoutModelTest::editWidgetTypeSwitchRelocatesWhenBlocked()
{
    DashboardLayoutModel model;
    // Value tile at (1,1); a blocker right next to it stops in-place growth to 4x3.
    const QString id =
        model.addWidget(QStringLiteral("value"), 1, QStringLiteral("rpm"), 1, 1, 1, 1);
    const QString blockerId =
        model.addWidget(QStringLiteral("value"), 2, QStringLiteral("rpm"), 2, 1, 1, 1);
    QVERIFY(!blockerId.isEmpty());

    QVERIFY(model.editWidget(id, QStringLiteral("chart"), 1, QStringLiteral("rpm")));

    // First free 4x3 spot scanning rows then columns: (3,1) (the old 1x1 slot
    // at (1,1) is freed by the move, but a 4x3 there would overlap the blocker).
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColSpanRole), 4);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowSpanRole), 3);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColRole), 3);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowRole), 1);
}

void DashboardLayoutModelTest::editWidgetTypeSwitchWithNoRoomIsRejected()
{
    DashboardLayoutModel model;
    const QString id =
        model.addWidget(QStringLiteral("value"), 1, QStringLiteral("rpm"), 1, 1, 1, 1);
    // Blocker covering everything except the value tile's cell.
    const QString blockerId =
        model.addWidget(QStringLiteral("chart"), 2, QStringLiteral("rpm"), 1, 2, 16, 19);
    QVERIFY(!blockerId.isEmpty());
    const QString blocker2Id =
        model.addWidget(QStringLiteral("chart"), 3, QStringLiteral("rpm"), 2, 1, 15, 1);
    QVERIFY(!blocker2Id.isEmpty());

    QVERIFY(!model.editWidget(id, QStringLiteral("chart"), 1, QStringLiteral("rpm")));

    // Card is unchanged.
    const QModelIndex index = model.index(0);
    QCOMPARE(model.data(index, DashboardLayoutModel::WidgetTypeRole).toString(),
             QStringLiteral("value"));
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColSpanRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowSpanRole), 1);
}

void DashboardLayoutModelTest::batterySummaryUsesMotorSummaryDefaults()
{
    DashboardLayoutModel model;

    QCOMPARE(model.defaultColSpan(QStringLiteral("batterySummary")),
             model.defaultColSpan(QStringLiteral("motorSummary")));
    QCOMPARE(model.defaultRowSpan(QStringLiteral("batterySummary")),
             model.defaultRowSpan(QStringLiteral("motorSummary")));

    const QString id = model.addWidget(QStringLiteral("batterySummary"), 0, QString(), 1, 1, 0, 0);
    QVERIFY(!id.isEmpty());
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowRole), 1);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::ColSpanRole), 3);
    QCOMPARE(widgetRow(model, id, DashboardLayoutModel::RowSpanRole), 4);
}

void DashboardLayoutModelTest::loadsExistingMotorOnlyLayoutUnchanged()
{
    QJsonArray widgets;
    QJsonObject widget;
    widget.insert(QStringLiteral("id"), QStringLiteral("motor-summary-1"));
    widget.insert(QStringLiteral("type"), QStringLiteral("motorSummary"));
    widget.insert(QStringLiteral("motorId"), 4);
    widget.insert(QStringLiteral("metric"), QString());
    widget.insert(QStringLiteral("col"), 2);
    widget.insert(QStringLiteral("row"), 3);
    widget.insert(QStringLiteral("colSpan"), 3);
    widget.insert(QStringLiteral("rowSpan"), 4);
    widgets.push_back(widget);

    QSettings settings;
    settings.beginGroup(QStringLiteral("Dashboard"));
    settings.setValue(QStringLiteral("widgetsJson"),
                      QString::fromUtf8(QJsonDocument(widgets).toJson(QJsonDocument::Compact)));
    settings.endGroup();
    settings.sync();

    DashboardLayoutModel model;
    QCOMPARE(model.rowCount(), 1);
    const QModelIndex index = model.index(0);
    QCOMPARE(model.data(index, DashboardLayoutModel::WidgetIdRole).toString(), QStringLiteral("motor-summary-1"));
    QCOMPARE(model.data(index, DashboardLayoutModel::WidgetTypeRole).toString(), QStringLiteral("motorSummary"));
    QCOMPARE(model.data(index, DashboardLayoutModel::MotorIdRole).toInt(), 4);
    QCOMPARE(model.data(index, DashboardLayoutModel::MetricRole).toString(), QString());
    QCOMPARE(model.data(index, DashboardLayoutModel::ColRole).toInt(), 2);
    QCOMPARE(model.data(index, DashboardLayoutModel::RowRole).toInt(), 3);
    QCOMPARE(model.data(index, DashboardLayoutModel::ColSpanRole).toInt(), 3);
    QCOMPARE(model.data(index, DashboardLayoutModel::RowSpanRole).toInt(), 4);
}

void DashboardLayoutModelTest::canPlaceReportsOverlapAndBounds()
{
    DashboardLayoutModel model;
    model.addWidget(QStringLiteral("chart"), 1, QStringLiteral("rpm"), 1, 1, 4, 3);

    QVERIFY(model.canPlace(5, 1, 4, 3));
    QVERIFY(!model.canPlace(2, 2, 4, 3));   // overlaps the chart
    QVERIFY(!model.canPlace(14, 1, 4, 3));  // off the right edge
    QVERIFY(!model.canPlace(1, 19, 1, 3));  // off the bottom edge
}

QTEST_GUILESS_MAIN(DashboardLayoutModelTest)

#include "DashboardLayoutModelTest.moc"
