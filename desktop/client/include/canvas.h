#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QProcess>

class Canvas : public QWidget
{
public:
    explicit Canvas(QWidget *parent = nullptr);
    ~Canvas() override;

    void setSpeed(int speed);
    void setTemperature(int temperature);
    void setBatteryLevel(int batteryLevel);
    void setLightSignals(bool leftLight, bool rightLight, bool warningLight);
    void setCommunicationStatus(bool connected, const QString &message);
    void setBlinkVisible(bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    enum class SignalSound
    {
        None,
        Left,
        Right,
        Warning
    };

    QColor temperatureColor() const;
    QColor batteryColor() const;
    bool hasActiveLightSignal() const;
    SignalSound selectedSignalSound() const;
    QString soundPathFor(SignalSound sound) const;
    void stopSignalSound();
    void startSignalSound();
    QPointF pointOnGauge(const QPointF &center, qreal radius, int speed) const;
    void drawGauge(QPainter &painter, const QRectF &rect) const;
    void drawNeedle(QPainter &painter, const QPointF &center, qreal radius) const;
    void drawSideIndicators(QPainter &painter, const QRectF &rect) const;
    void drawCommunication(QPainter &painter, const QRectF &rect) const;

    QString iconFontFamily_{"Material Icons"};
    int speed_{110};
    int temperature_{15};
    int batteryLevel_{55};
    bool leftLight_{false};
    bool rightLight_{false};
    bool warningLight_{false};
    bool connected_{true};
    bool blinkVisible_{true};
    QString communicationMessage_{"Connected"};
    QString leftSignalSoundPath_;
    QString rightSignalSoundPath_;
    QString warningSignalSoundPath_;
    QString signalSoundCommand_;
    QProcess signalSoundProcess_;
    SignalSound activeSignalSound_{SignalSound::None};
    bool stoppingSignalSound_{false};
};

#endif
