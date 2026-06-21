#pragma once

#include <QObject>
#include <QString>

extern "C" {
#include "canard.h"
}

class ICanTransport : public QObject
{
    Q_OBJECT

public:
    explicit ICanTransport(QObject *parent = nullptr) : QObject(parent) {}
    ~ICanTransport() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void canFrameReceived(const CanardCANFrame &frame);
    void opened(const QString &endpointName);
    void openFailed();
};
