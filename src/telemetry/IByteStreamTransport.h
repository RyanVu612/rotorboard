#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

class IByteStreamTransport : public QObject
{
    Q_OBJECT

public:
    explicit IByteStreamTransport(QObject *parent = nullptr) : QObject(parent) {}
    ~IByteStreamTransport() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void bytesReceived(const QByteArray &data);
    void opened(const QString &endpointName);
    void openFailed();
};
