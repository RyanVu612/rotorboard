#include "store/DashboardLayoutModel.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QSettings>
#include <QtTest>

#include <memory>

class TestTelemetryModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sampleRevision READ sampleRevision NOTIFY sampleRevisionChanged)

public:
    explicit TestTelemetryModel(QObject *parent = nullptr) : QObject(parent) {}

    int sampleRevision() const { return m_sampleRevision; }

    Q_INVOKABLE int rowCount() const { return m_motorIds.size(); }
    Q_INVOKABLE int motorIdAt(int row) const { return row >= 0 && row < m_motorIds.size() ? m_motorIds.at(row) : 0; }
    Q_INVOKABLE QVariant valueForMetric(int, const QString &) const { return 0.0; }
    Q_INVOKABLE QVariantList historyForMetric(int, const QString &) const { return {}; }
    Q_INVOKABLE bool isMotorStale(int) const { return true; }
    Q_INVOKABLE int warningLevelForMotor(int) const { return 0; }
    Q_INVOKABLE QString statusForMotor(int) const { return QString(); }

    void setMotorIds(const QVector<int> &motorIds)
    {
        m_motorIds = motorIds;
        ++m_sampleRevision;
        emit sampleRevisionChanged();
        emit modelReset();
    }

signals:
    void sampleRevisionChanged();
    void rowsInserted();
    void modelReset();
    void motorHistoryChanged(int motorId);

private:
    int m_sampleRevision = 0;
    QVector<int> m_motorIds;
};

class TestController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject *telemetryModel READ telemetryModel CONSTANT)
    Q_PROPERTY(QObject *layoutModel READ layoutModel CONSTANT)
    Q_PROPERTY(QString sourceLabel READ sourceLabel NOTIFY sourceLabelChanged)
    Q_PROPERTY(bool linkMonitored READ linkMonitored NOTIFY linkStatusChanged)
    Q_PROPERTY(int linkStateLevel READ linkStateLevel NOTIFY linkStatusChanged)
    Q_PROPERTY(QString linkStatusText READ linkStatusText NOTIFY linkStatusChanged)
    Q_PROPERTY(bool chartsFrozen READ chartsFrozen WRITE setChartsFrozen NOTIFY chartsFrozenChanged)
    Q_PROPERTY(QString inputMethod READ inputMethod WRITE setInputMethod NOTIFY inputSettingsChanged)
    Q_PROPERTY(QString playbackPath READ playbackPath WRITE setPlaybackPath NOTIFY inputSettingsChanged)
    Q_PROPERTY(QString mavlinkSerialPort READ mavlinkSerialPort WRITE setMavlinkSerialPort NOTIFY inputSettingsChanged)
    Q_PROPERTY(int mavlinkSerialBaud READ mavlinkSerialBaud WRITE setMavlinkSerialBaud NOTIFY inputSettingsChanged)

public:
    explicit TestController(QObject *parent = nullptr) : QObject(parent) {}

    QObject *telemetryModel() { return &m_telemetryModel; }
    QObject *layoutModel() { return &m_layoutModel; }
    DashboardLayoutModel *layout() { return &m_layoutModel; }
    TestTelemetryModel *telemetry() { return &m_telemetryModel; }

    QString sourceLabel() const { return QStringLiteral("Test"); }
    bool linkMonitored() const { return false; }
    int linkStateLevel() const { return 3; }
    QString linkStatusText() const { return QString(); }

    bool chartsFrozen() const { return m_chartsFrozen; }
    void setChartsFrozen(bool frozen)
    {
        if (m_chartsFrozen == frozen) {
            return;
        }
        m_chartsFrozen = frozen;
        emit chartsFrozenChanged();
    }

    QString inputMethod() const { return m_inputMethod; }
    void setInputMethod(const QString &method)
    {
        m_inputMethod = method;
        emit inputSettingsChanged();
    }

    QString playbackPath() const { return m_playbackPath; }
    void setPlaybackPath(const QString &path)
    {
        m_playbackPath = path;
        emit inputSettingsChanged();
    }

    QString mavlinkSerialPort() const { return m_mavlinkSerialPort; }
    void setMavlinkSerialPort(const QString &port)
    {
        m_mavlinkSerialPort = port;
        emit inputSettingsChanged();
    }

    int mavlinkSerialBaud() const { return m_mavlinkSerialBaud; }
    void setMavlinkSerialBaud(int baud)
    {
        m_mavlinkSerialBaud = baud;
        emit inputSettingsChanged();
    }

    Q_INVOKABLE void toggleChartsFrozen() { setChartsFrozen(!m_chartsFrozen); }
    Q_INVOKABLE void applyTelemetrySource() {}

signals:
    void chartsFrozenChanged();
    void sourceLabelChanged();
    void linkStatusChanged();
    void inputSettingsChanged();

private:
    bool m_chartsFrozen = false;
    QString m_inputMethod = QStringLiteral("fake");
    QString m_playbackPath;
    QString m_mavlinkSerialPort;
    int m_mavlinkSerialBaud = 115200;
    TestTelemetryModel m_telemetryModel;
    DashboardLayoutModel m_layoutModel;
};

class DashboardIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void cleanup();

    void pageLoadsWithEmptyLayout();
    void addDialogAndGhostPlacement();
    void ghostRejectsOccupiedAndCancels();
    void addWhileGhostActiveReopensDialog();
    void dragResizeAndSwapUpdateLayout();
    void contextMenuEditDuplicateDelete();
    void pauseToggleAndTelemetrySeeding();

private:
    struct Harness {
        TestController controller;
        QQuickView view;
        QQuickItem *page = nullptr;
    };

    std::unique_ptr<Harness> createHarness();
    QObject *find(QObject *root, const QString &name);
    QQuickItem *findItem(QObject *root, const QString &name);
    QQuickItem *findSlot(QObject *root, const QString &widgetId);
    QString addWidget(DashboardLayoutModel *model,
                      const QString &type,
                      int motorId,
                      const QString &metric,
                      int col,
                      int row,
                      int colSpan,
                      int rowSpan);
    QVariant roleData(DashboardLayoutModel *model, const QString &widgetId, DashboardLayoutModel::Role role);
    bool hasWidget(DashboardLayoutModel *model, const QString &widgetId);
    void waitForQml();
    void invoke(QObject *object, const char *method);
    void setComboIndex(QObject *combo, int index);
    void setSpinValue(QObject *spin, int value);

    QStringList m_qmlWarnings;
    static DashboardIntegrationTest *s_activeTest;
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);
};

DashboardIntegrationTest *DashboardIntegrationTest::s_activeTest = nullptr;

void DashboardIntegrationTest::messageHandler(QtMsgType type,
                                              const QMessageLogContext &context,
                                              const QString &message)
{
    if (s_activeTest && type == QtWarningMsg) {
        const QString file = QString::fromUtf8(context.file ? context.file : "");
        if (file.contains(QStringLiteral("/qml/")) || message.contains(QStringLiteral("Dashboard")) ||
            message.contains(QStringLiteral("Widget")) || message.contains(QStringLiteral("qrc:/"))) {
            s_activeTest->m_qmlWarnings.append(message);
        }
    }
    fprintf(stderr, "%s\n", qPrintable(message));
}

void DashboardIntegrationTest::initTestCase()
{
    s_activeTest = this;
    qInstallMessageHandler(DashboardIntegrationTest::messageHandler);
    QCoreApplication::setOrganizationName(QStringLiteral("rotorboard-test"));
    QCoreApplication::setApplicationName(QStringLiteral("dashboard-integration-test"));
}

void DashboardIntegrationTest::init()
{
    m_qmlWarnings.clear();
    QSettings settings;
    settings.clear();
    settings.sync();
}

void DashboardIntegrationTest::cleanup()
{
    QVERIFY2(m_qmlWarnings.isEmpty(), qPrintable(QStringLiteral("Unexpected QML warning: %1").arg(m_qmlWarnings.join(QStringLiteral("\n")))));
}

std::unique_ptr<DashboardIntegrationTest::Harness> DashboardIntegrationTest::createHarness()
{
    auto harness = std::make_unique<Harness>();
    harness->view.engine()->addImportPath(QStringLiteral("qrc:/qt/qml"));
    harness->view.resize(1120, 720);
    harness->view.setGeometry(0, 0, 1120, 720);
    harness->view.setResizeMode(QQuickView::SizeRootObjectToView);
    QVariantMap properties;
    properties.insert(QStringLiteral("controller"), QVariant::fromValue<QObject *>(&harness->controller));
    harness->view.setInitialProperties(properties);
    harness->view.setSource(QUrl(QStringLiteral("qrc:/qt/qml/Rotorboard/DashboardPage.qml")));
    if (harness->view.status() == QQuickView::Error) {
        QStringList errors;
        for (const QQmlError &error : harness->view.errors()) {
            errors.append(error.toString());
        }
        QTest::qFail(qPrintable(errors.join(QStringLiteral("\n"))), __FILE__, __LINE__);
        return nullptr;
    }

    harness->page = qobject_cast<QQuickItem *>(harness->view.rootObject());
    if (!harness->page) {
        QTest::qFail("DashboardPage did not create a QQuickItem", __FILE__, __LINE__);
        return nullptr;
    }
    harness->view.show();
    if (!QTest::qWaitForWindowExposed(&harness->view)) {
        QTest::qFail("Dashboard test window was not exposed", __FILE__, __LINE__);
        return nullptr;
    }
    waitForQml();
    return harness;
}

QObject *DashboardIntegrationTest::find(QObject *root, const QString &name)
{
    QObject *object = root->findChild<QObject *>(name);
    if (!object) {
        QTest::qFail(qPrintable(QStringLiteral("Missing objectName '%1'").arg(name)), __FILE__, __LINE__);
        return nullptr;
    }
    return object;
}

QQuickItem *DashboardIntegrationTest::findItem(QObject *root, const QString &name)
{
    QQuickItem *item = root->findChild<QQuickItem *>(name);
    if (!item) {
        QTest::qFail(qPrintable(QStringLiteral("Missing item objectName '%1'").arg(name)), __FILE__, __LINE__);
        return nullptr;
    }
    return item;
}

QQuickItem *DashboardIntegrationTest::findSlot(QObject *root, const QString &widgetId)
{
    QQuickItem *rootItem = qobject_cast<QQuickItem *>(root);
    for (int attempt = 0; attempt < 25; ++attempt) {
        QVector<QQuickItem *> stack;
        if (rootItem) {
            stack.append(rootItem);
        }
        while (!stack.isEmpty()) {
            QQuickItem *item = stack.takeLast();
            if (item->property("widgetId").toString() == widgetId) {
                return item;
            }
            const QList<QQuickItem *> children = item->childItems();
            for (QQuickItem *child : children) {
                stack.append(child);
            }
        }
        waitForQml();
    }
    QTest::qFail(qPrintable(QStringLiteral("Missing widget slot for id '%1'").arg(widgetId)), __FILE__, __LINE__);
    return nullptr;
}

QString DashboardIntegrationTest::addWidget(DashboardLayoutModel *model,
                                            const QString &type,
                                            int motorId,
                                            const QString &metric,
                                            int col,
                                            int row,
                                            int colSpan,
                                            int rowSpan)
{
    const QString id = model->addWidget(type, motorId, metric, col, row, colSpan, rowSpan);
    if (id.isEmpty()) {
        QTest::qFail("Failed to add widget in test setup", __FILE__, __LINE__);
        return {};
    }
    waitForQml();
    return id;
}

QVariant DashboardIntegrationTest::roleData(DashboardLayoutModel *model,
                                            const QString &widgetId,
                                            DashboardLayoutModel::Role role)
{
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex index = model->index(row);
        if (model->data(index, DashboardLayoutModel::WidgetIdRole).toString() == widgetId) {
            return model->data(index, role);
        }
    }
    QTest::qFail(qPrintable(QStringLiteral("Missing widget id '%1'").arg(widgetId)), __FILE__, __LINE__);
    return {};
}

bool DashboardIntegrationTest::hasWidget(DashboardLayoutModel *model, const QString &widgetId)
{
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex index = model->index(row);
        if (model->data(index, DashboardLayoutModel::WidgetIdRole).toString() == widgetId) {
            return true;
        }
    }
    return false;
}

void DashboardIntegrationTest::waitForQml()
{
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    QTest::qWait(20);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
}

void DashboardIntegrationTest::invoke(QObject *object, const char *method)
{
    QVERIFY(QMetaObject::invokeMethod(object, method));
    waitForQml();
}

void DashboardIntegrationTest::setComboIndex(QObject *combo, int index)
{
    QVERIFY(combo->setProperty("currentIndex", index));
    waitForQml();
}

void DashboardIntegrationTest::setSpinValue(QObject *spin, int value)
{
    QVERIFY(spin->setProperty("value", value));
    waitForQml();
}

void DashboardIntegrationTest::pageLoadsWithEmptyLayout()
{
    auto harness = createHarness();
    QVERIFY(harness);
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));
    QObject *placeholder = find(harness->page, QStringLiteral("emptyDashboardPlaceholder"));

    QCOMPARE(harness->controller.layout()->rowCount(), 0);
    QVERIFY(placeholder->property("visible").toBool());
    QCOMPARE(grid->property("ghostActive").toBool(), false);
}

void DashboardIntegrationTest::addDialogAndGhostPlacement()
{
    auto harness = createHarness();
    QVERIFY(harness);
    QObject *dialog = find(harness->page, QStringLiteral("widgetDialog"));
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));
    QObject *typeCombo = find(dialog, QStringLiteral("widgetTypeCombo"));
    QObject *motorSpin = find(dialog, QStringLiteral("widgetMotorSpinBox"));
    QObject *metricCombo = find(dialog, QStringLiteral("widgetMetricCombo"));

    invoke(dialog, "openForAdd");
    QVERIFY(dialog->property("logicallyOpen").toBool());
    QCOMPARE(dialog->property("mode").toString(), QStringLiteral("add"));
    QCOMPARE(typeCombo->property("currentIndex").toInt(), 0);
    QCOMPARE(motorSpin->property("value").toInt(), 1);
    QCOMPARE(metricCombo->property("currentText").toString(), QStringLiteral("rpm"));

    setComboIndex(typeCombo, 1);
    setSpinValue(motorSpin, 3);
    setComboIndex(metricCombo, 1);
    QVERIFY(QMetaObject::invokeMethod(dialog, "confirmed",
                                      Q_ARG(QString, QStringLiteral("add")),
                                      Q_ARG(QString, QString()),
                                      Q_ARG(QString, QStringLiteral("chart")),
                                      Q_ARG(int, 3),
                                      Q_ARG(QString, QStringLiteral("voltage"))));
    invoke(dialog, "close");

    QVERIFY(grid->property("ghostActive").toBool());
    QCOMPARE(grid->property("ghostType").toString(), QStringLiteral("chart"));
    QCOMPARE(grid->property("ghostMotorId").toInt(), 3);
    QCOMPARE(grid->property("ghostMetric").toString(), QStringLiteral("voltage"));
    QCOMPARE(grid->property("ghostColSpan").toInt(), 4);
    QCOMPARE(grid->property("ghostRowSpan").toInt(), 3);

    QVariant returned;
    QVERIFY(QMetaObject::invokeMethod(grid, "updateGhost", Q_RETURN_ARG(QVariant, returned),
                                      Q_ARG(QVariant, 180), Q_ARG(QVariant, 90)));
    waitForQml();
    QCOMPARE(grid->property("ghostCol").toInt(), 3);
    QCOMPARE(grid->property("ghostRow").toInt(), 2);
    QVERIFY(grid->property("ghostVisible").toBool());
    QVERIFY(find(harness->page, QStringLiteral("ghostPreview"))->property("placementValid").toBool());

    invoke(grid, "commitGhost");
    QCOMPARE(harness->controller.layout()->rowCount(), 1);
    QCOMPARE(grid->property("ghostActive").toBool(), false);
    const QModelIndex index = harness->controller.layout()->index(0);
    QCOMPARE(harness->controller.layout()->data(index, DashboardLayoutModel::WidgetTypeRole).toString(), QStringLiteral("chart"));
    QCOMPARE(harness->controller.layout()->data(index, DashboardLayoutModel::MotorIdRole).toInt(), 3);
    QCOMPARE(harness->controller.layout()->data(index, DashboardLayoutModel::MetricRole).toString(), QStringLiteral("voltage"));
    QCOMPARE(harness->controller.layout()->data(index, DashboardLayoutModel::ColRole).toInt(), 3);
    QCOMPARE(harness->controller.layout()->data(index, DashboardLayoutModel::RowRole).toInt(), 2);
}

void DashboardIntegrationTest::ghostRejectsOccupiedAndCancels()
{
    auto harness = createHarness();
    QVERIFY(harness);
    DashboardLayoutModel *layout = harness->controller.layout();
    addWidget(layout, QStringLiteral("chart"), 1, QStringLiteral("rpm"), 1, 1, 4, 3);
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));

    QVERIFY(QMetaObject::invokeMethod(grid, "startGhost", Q_ARG(QVariant, QStringLiteral("chart")),
                                      Q_ARG(QVariant, 2), Q_ARG(QVariant, QStringLiteral("rpm"))));
    QVERIFY(QMetaObject::invokeMethod(grid, "updateGhost", Q_ARG(QVariant, 10), Q_ARG(QVariant, 10)));
    waitForQml();
    QVERIFY(!find(harness->page, QStringLiteral("ghostPreview"))->property("placementValid").toBool());
    invoke(grid, "commitGhost");
    QCOMPARE(layout->rowCount(), 1);
    QVERIFY(grid->property("ghostActive").toBool());

    invoke(grid, "cancelGhost");
    QCOMPARE(grid->property("ghostActive").toBool(), false);

    QVERIFY(QMetaObject::invokeMethod(grid, "startGhost", Q_ARG(QVariant, QStringLiteral("value")),
                                      Q_ARG(QVariant, 2), Q_ARG(QVariant, QStringLiteral("rpm"))));
    waitForQml();
    QTest::keyClick(&harness->view, Qt::Key_Escape);
    waitForQml();
    QCOMPARE(grid->property("ghostActive").toBool(), false);
    QCOMPARE(layout->rowCount(), 1);

    QVERIFY(QMetaObject::invokeMethod(grid, "startGhost", Q_ARG(QVariant, QStringLiteral("value")),
                                      Q_ARG(QVariant, 2), Q_ARG(QVariant, QStringLiteral("rpm"))));
    waitForQml();
    QQuickItem *overlay = findItem(harness->page, QStringLiteral("ghostOverlay"));
    QTest::mouseClick(&harness->view, Qt::RightButton, Qt::NoModifier,
                      overlay->mapToScene(QPointF(20, 20)).toPoint());
    waitForQml();
    QCOMPARE(grid->property("ghostActive").toBool(), false);

    QVERIFY(QMetaObject::invokeMethod(grid, "startGhost", Q_ARG(QVariant, QStringLiteral("value")),
                                      Q_ARG(QVariant, 2), Q_ARG(QVariant, QStringLiteral("rpm"))));
    waitForQml();
    QTest::mouseClick(&harness->view, Qt::LeftButton, Qt::NoModifier, QPoint(8, 8));
    waitForQml();
    QCOMPARE(grid->property("ghostActive").toBool(), false);
    QCOMPARE(layout->rowCount(), 1);
}

void DashboardIntegrationTest::addWhileGhostActiveReopensDialog()
{
    auto harness = createHarness();
    QVERIFY(harness);
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));
    QObject *dialog = find(harness->page, QStringLiteral("widgetDialog"));
    QVERIFY(QMetaObject::invokeMethod(grid, "startGhost", Q_ARG(QVariant, QStringLiteral("value")),
                                      Q_ARG(QVariant, 2), Q_ARG(QVariant, QStringLiteral("rpm"))));
    waitForQml();
    QVERIFY(grid->property("ghostActive").toBool());

    QQuickItem *addButton = findItem(harness->page, QStringLiteral("addButton"));
    QTest::mouseClick(&harness->view, Qt::LeftButton, Qt::NoModifier,
                      addButton->mapToScene(QPointF(addButton->width() / 2, addButton->height() / 2)).toPoint());
    waitForQml();
    QVERIFY(!grid->property("ghostActive").toBool());
    QVERIFY(dialog->property("logicallyOpen").toBool());
    QCOMPARE(dialog->property("mode").toString(), QStringLiteral("add"));
}

void DashboardIntegrationTest::dragResizeAndSwapUpdateLayout()
{
    auto harness = createHarness();
    QVERIFY(harness);
    DashboardLayoutModel *layout = harness->controller.layout();
    const QString first = addWidget(layout, QStringLiteral("value"), 1, QStringLiteral("rpm"), 1, 1, 1, 1);
    const QString second = addWidget(layout, QStringLiteral("value"), 2, QStringLiteral("rpm"), 2, 1, 1, 1);
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));

    QQuickItem *slot = findSlot(harness->page, first);
    QVERIFY(slot);
    QQuickItem *body = slot->findChild<QQuickItem *>(QStringLiteral("widgetSlotBodyArea"));
    QVERIFY(body);
    const QPoint start = body->mapToScene(QPointF(20, 20)).toPoint();
    QTest::mousePress(&harness->view, Qt::LeftButton, Qt::NoModifier, start);
    waitForQml();
    QVERIFY(slot->property("dragging").toBool());
    QVERIFY(grid->property("dragLockCount").toInt() > 0);
    QVERIFY(!grid->property("interactive").toBool());

    QTest::mouseMove(&harness->view, start + QPoint(90, 0));
    QTest::mouseRelease(&harness->view, Qt::LeftButton, Qt::NoModifier, start + QPoint(90, 0));
    waitForQml();
    QCOMPARE(grid->property("dragLockCount").toInt(), 0);
    QVERIFY(grid->property("interactive").toBool());
    QCOMPARE(roleData(layout, first, DashboardLayoutModel::ColRole).toInt(), 2);
    QCOMPARE(roleData(layout, second, DashboardLayoutModel::ColRole).toInt(), 1);

    QQuickItem *resize = slot->findChild<QQuickItem *>(QStringLiteral("widgetSlotResizeArea"));
    QVERIFY(resize);
    const QPoint resizeStart = resize->mapToScene(QPointF(8, 8)).toPoint();
    QTest::mousePress(&harness->view, Qt::LeftButton, Qt::NoModifier, resizeStart);
    QTest::mouseMove(&harness->view, resizeStart + QPoint(190, 80));
    QTest::mouseRelease(&harness->view, Qt::LeftButton, Qt::NoModifier, resizeStart + QPoint(190, 80));
    waitForQml();
    QVERIFY(roleData(layout, first, DashboardLayoutModel::ColSpanRole).toInt() >= 2);
    QVERIFY(roleData(layout, first, DashboardLayoutModel::RowSpanRole).toInt() >= 2);

    const int spanBefore = roleData(layout, first, DashboardLayoutModel::ColSpanRole).toInt();
    layout->resizeWidget(first, 16, 20);
    waitForQml();
    QCOMPARE(roleData(layout, first, DashboardLayoutModel::ColSpanRole).toInt(), spanBefore);
}

void DashboardIntegrationTest::contextMenuEditDuplicateDelete()
{
    auto harness = createHarness();
    QVERIFY(harness);
    DashboardLayoutModel *layout = harness->controller.layout();
    const QString id = addWidget(layout, QStringLiteral("value"), 4, QStringLiteral("rpm"), 1, 1, 1, 1);
    QObject *grid = find(harness->page, QStringLiteral("dashboardGrid"));
    QObject *dialog = find(harness->page, QStringLiteral("widgetDialog"));

    QVERIFY(QMetaObject::invokeMethod(grid, "openWidgetMenu", Q_ARG(QVariant, id),
                                      Q_ARG(QVariant, QStringLiteral("value")),
                                      Q_ARG(QVariant, 4),
                                      Q_ARG(QVariant, QStringLiteral("rpm")),
                                      Q_ARG(QVariant, QPointF(20, 20))));
    waitForQml();
    QObject *menu = find(harness->page, QStringLiteral("cardMenu"));
    QVERIFY(menu->property("opened").toBool());
    QCOMPARE(find(harness->page, QStringLiteral("cardMenuEditAction"))->property("text").toString(), QStringLiteral("Edit…"));
    QCOMPARE(find(harness->page, QStringLiteral("cardMenuDuplicateAction"))->property("text").toString(), QStringLiteral("Duplicate"));
    QCOMPARE(find(harness->page, QStringLiteral("cardMenuDeleteAction"))->property("text").toString(), QStringLiteral("Delete"));

    invoke(find(harness->page, QStringLiteral("cardMenuEditAction")), "triggered");
    QVERIFY(dialog->property("logicallyOpen").toBool());
    QCOMPARE(dialog->property("mode").toString(), QStringLiteral("edit"));
    QCOMPARE(dialog->property("widgetId").toString(), id);
    QCOMPARE(find(dialog, QStringLiteral("widgetMotorSpinBox"))->property("value").toInt(), 4);
    QCOMPARE(find(dialog, QStringLiteral("widgetMetricCombo"))->property("currentText").toString(), QStringLiteral("rpm"));

    QObject *typeCombo = find(dialog, QStringLiteral("widgetTypeCombo"));
    QObject *metricCombo = find(dialog, QStringLiteral("widgetMetricCombo"));
    setComboIndex(typeCombo, 2);
    QVERIFY(!metricCombo->property("visible").toBool());
    QVERIFY(QMetaObject::invokeMethod(dialog, "confirmed",
                                      Q_ARG(QString, QStringLiteral("edit")),
                                      Q_ARG(QString, id),
                                      Q_ARG(QString, QStringLiteral("motorSummary")),
                                      Q_ARG(int, 4),
                                      Q_ARG(QString, QString())));
    invoke(dialog, "close");
    QCOMPARE(layout->rowCount(), 1);
    QCOMPARE(roleData(layout, id, DashboardLayoutModel::WidgetTypeRole).toString(), QStringLiteral("motorSummary"));
    QCOMPARE(roleData(layout, id, DashboardLayoutModel::MetricRole).toString(), QString());

    QVERIFY(QMetaObject::invokeMethod(grid, "openWidgetMenu", Q_ARG(QVariant, id),
                                      Q_ARG(QVariant, QStringLiteral("motorSummary")),
                                      Q_ARG(QVariant, 4),
                                      Q_ARG(QVariant, QString()),
                                      Q_ARG(QVariant, QPointF(20, 20))));
    waitForQml();
    invoke(find(harness->page, QStringLiteral("cardMenuDuplicateAction")), "triggered");
    QCOMPARE(layout->rowCount(), 2);

    QSettings settings;
    settings.clear();
    settings.sync();
    DashboardLayoutModel fullModel;
    const QString full = fullModel.addWidget(QStringLiteral("chart"), 9, QStringLiteral("rpm"), 1, 1, 16, 20);
    QVERIFY(!full.isEmpty());
    QCOMPARE(fullModel.duplicateWidget(full), QString());

    QVERIFY(QMetaObject::invokeMethod(grid, "openWidgetMenu", Q_ARG(QVariant, id),
                                      Q_ARG(QVariant, QStringLiteral("motorSummary")),
                                      Q_ARG(QVariant, 4),
                                      Q_ARG(QVariant, QString()),
                                      Q_ARG(QVariant, QPointF(20, 20))));
    waitForQml();
    invoke(find(harness->page, QStringLiteral("cardMenuDeleteAction")), "triggered");
    QVERIFY(!hasWidget(layout, id));
}

void DashboardIntegrationTest::pauseToggleAndTelemetrySeeding()
{
    auto harness = createHarness();
    QVERIFY(harness);
    QObject *pauseLabel = find(harness->page, QStringLiteral("pauseLabel"));
    QCOMPARE(pauseLabel->property("text").toString(), QStringLiteral("Pause"));

    QQuickItem *pauseButton = findItem(harness->page, QStringLiteral("pauseButton"));
    QTest::mouseClick(&harness->view, Qt::LeftButton, Qt::NoModifier,
                      pauseButton->mapToScene(QPointF(pauseButton->width() / 2, pauseButton->height() / 2)).toPoint());
    waitForQml();
    QVERIFY(harness->controller.chartsFrozen());
    QCOMPARE(pauseLabel->property("text").toString(), QStringLiteral("Resume"));

    const QString valueId = addWidget(harness->controller.layout(), QStringLiteral("value"), 1, QStringLiteral("rpm"), 1, 1, 1, 1);
    const QString chartId = addWidget(harness->controller.layout(), QStringLiteral("chart"), 2, QStringLiteral("rpm"), 2, 1, 4, 3);
    const QString summaryId = addWidget(harness->controller.layout(), QStringLiteral("motorSummary"), 3, QString(), 6, 1, 4, 2);
    QVERIFY(findSlot(harness->page, valueId));
    QVERIFY(findSlot(harness->page, chartId));
    QVERIFY(findSlot(harness->page, summaryId));

    harness->controller.telemetry()->setMotorIds({11});
    waitForQml();
    QVERIFY(harness->controller.layout()->hasWidgetsForMotor(11));
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QGuiApplication app(argc, argv);
    DashboardIntegrationTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "DashboardIntegrationTest.moc"
