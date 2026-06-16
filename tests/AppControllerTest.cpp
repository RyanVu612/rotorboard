#include "app/AppController.h"
#include "store/MotorTelemetryModel.h"

#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>

class AppControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void switchesInputMethodAndUpdatesSourceState();
    void switchingWhileRunningStartsReplacementSource();
    void explicitUdpMavlinkSourceIsPreserved();
    void loadsSavedSettingsWhenNoExplicitSourceIsProvided();
};

void AppControllerTest::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("RotorboardTests"));
    QCoreApplication::setApplicationName(QStringLiteral("AppControllerTest"));
    QSettings().clear();
}

void AppControllerTest::switchesInputMethodAndUpdatesSourceState()
{
    SourceConfig config;
    config.kind = SourceKind::Fake;
    AppController controller(config, true);

    QCOMPARE(controller.inputMethod(), QStringLiteral("fake"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("Fake source"));
    QCOMPARE(controller.linkMonitored(), false);

    QSignalSpy sourceLabelSpy(&controller, &AppController::sourceLabelChanged);
    QSignalSpy linkStatusSpy(&controller, &AppController::linkStatusChanged);

    controller.setInputMethod(QStringLiteral("playback"));
    controller.setPlaybackPath(QStringLiteral("samples/session.csv"));
    controller.applyTelemetrySource();

    QCOMPARE(controller.inputMethod(), QStringLiteral("playback"));
    QCOMPARE(controller.playbackPath(), QStringLiteral("samples/session.csv"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("Playback"));
    QCOMPARE(controller.linkMonitored(), false);

    controller.setInputMethod(QStringLiteral("pixhawk"));
    controller.setMavlinkSerialPort(QStringLiteral("/dev/tty.usbmodem01"));
    controller.setMavlinkSerialBaud(57600);
    controller.applyTelemetrySource();

    QCOMPARE(controller.inputMethod(), QStringLiteral("pixhawk"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("MAVLink serial /dev/tty.usbmodem01"));
    QCOMPARE(controller.linkMonitored(), true);
    QCOMPARE(controller.linkStatusText(), QStringLiteral("waiting for data"));

    controller.setInputMethod(QStringLiteral("fake"));
    controller.applyTelemetrySource();

    QCOMPARE(controller.inputMethod(), QStringLiteral("fake"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("Fake source"));
    QCOMPARE(controller.linkMonitored(), false);
    QCOMPARE(controller.linkStatusText(), QString());
    QVERIFY(sourceLabelSpy.count() >= 3);
    QVERIFY(linkStatusSpy.count() >= 3);
}

void AppControllerTest::switchingWhileRunningStartsReplacementSource()
{
    SourceConfig config;
    config.kind = SourceKind::Fake;
    AppController controller(config, true);
    auto *model = qobject_cast<MotorTelemetryModel *>(controller.telemetryModel());
    QVERIFY(model != nullptr);

    controller.start();
    controller.setInputMethod(QStringLiteral("playback"));
    controller.setPlaybackPath(QStringLiteral(ROTORBOARD_SOURCE_DIR "/samples/session.csv"));
    controller.applyTelemetrySource();

    QTRY_VERIFY(model->rowCount() > 0);
    QCOMPARE(controller.sourceLabel(), QStringLiteral("Playback"));
    controller.stop();
}

void AppControllerTest::explicitUdpMavlinkSourceIsPreserved()
{
    SourceConfig config;
    config.kind = SourceKind::Mavlink;
    config.mavlinkHost = QStringLiteral("127.0.0.1");
    config.mavlinkPort = 14551;

    AppController controller(config, true);

    QCOMPARE(controller.inputMethod(), QStringLiteral("pixhawk"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("MAVLink UDP 127.0.0.1:14551"));
    QCOMPARE(controller.linkMonitored(), false);
}

void AppControllerTest::loadsSavedSettingsWhenNoExplicitSourceIsProvided()
{
    QSettings settings;
    settings.clear();
    settings.beginGroup(QStringLiteral("TelemetryInput"));
    settings.setValue(QStringLiteral("inputMethod"), QStringLiteral("playback"));
    settings.setValue(QStringLiteral("playbackPath"), QStringLiteral("samples/session.csv"));
    settings.endGroup();

    AppController controller(SourceConfig(), false);

    QCOMPARE(controller.inputMethod(), QStringLiteral("playback"));
    QCOMPARE(controller.playbackPath(), QStringLiteral("samples/session.csv"));
    QCOMPARE(controller.sourceLabel(), QStringLiteral("Playback"));
}

QTEST_MAIN(AppControllerTest)
#include "AppControllerTest.moc"
