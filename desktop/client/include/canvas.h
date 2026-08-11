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

    // * === Methods === *
    // Setter methods
    void setSpeed(int speed);
    void setBlinkVisible(bool visible);
    void setTemperature(int temperature);
    void setBatteryLevel(int batteryLevel);
    void setCommunicationStatus(bool connected, const QString &message);
    void setLightSignals(bool leftLight, bool rightLight, bool warningLight);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    // * === Types === *
    enum class SignalSound
    {
        None,
        Left,
        Right,
        Warning
    };

    // * === Variables === *
    // Misc
    bool m_connected{true};
    bool m_blinkVisible{true};

    // Fonts
    QString m_iconFontFamily{"Material Icons"};

    // Messages
    QString m_communicationMessage{"Connected"};

    // Signals
    int m_speed{110};
    int m_temperature{15};
    int m_batteryLevel{55};
    bool m_leftLight{false};
    bool m_rightLight{false};
    bool m_warningLight{false};

    // Sounds
    QString m_signalSoundCommand;
    QProcess m_signalSoundProcess;
    QString m_leftSignalSoundPath;
    QString m_rightSignalSoundPath;
    QString m_warningSignalSoundPath;
    bool m_stoppingSignalSound{false};
    SignalSound m_activeSignalSound{SignalSound::None};

    // Icons
    const ushort m_ErrorIcon = 0xe628;
    const ushort m_SpeedIcon = 0xe9e4;
    const ushort m_BatteryIcon = 0xebdc;
    const ushort m_LeftArrowIcon = 0xe5c4;
    const ushort m_RightArrowIcon = 0xe5c8;
    const ushort m_TemperatureIcon = 0xe1ff;

    // * === Methods === *
    // Sound methods
    void stopSignalSound();
    void startSignalSound();
    SignalSound selectedSignalSound() const;

    // Color methods
    QColor batteryColor() const;
    QColor temperatureColor() const;

    // Draw methods
    void drawGauge(QPainter &painter, const QRectF &rect) const;
    void drawCommunication(QPainter &painter, const QRectF &rect) const;
    void drawSideIndicators(QPainter &painter, const QRectF &rect) const;
    QPointF pointOnGauge(const QPointF &center, qreal radius, int speed) const;
    void drawNeedle(QPainter &painter, const QPointF &center, qreal radius) const;

    // Misc methods
    bool hasActiveLightSignal() const;
    QString soundPathFor(SignalSound sound) const;
};

#endif
