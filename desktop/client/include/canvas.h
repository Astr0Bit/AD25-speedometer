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

    // * === Variables === *
    const int m_blink_interval_ms{330};

    // * === Methods === *
    // Setter methods
    void setSpeed(int speed);
    void setBlinkVisible(bool visible);
    void setTemperature(int temperature);
    void setBatteryLevel(int batteryLevel);
    void setCommunicationStatus(bool connected);
    void setLightSignals(bool leftLight, bool rightLight);

protected:
    // Events
    void paintEvent(QPaintEvent *event) override;

    // To set the default window size
    QSize sizeHint() const override;

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
    const int m_waitTime_ms{100};

    // Colors
    const QColor m_tempRed{235, 76, 92};
    const QColor m_tempBlue{48, 150, 255};
    const QColor m_tempWhite{245, 247, 250};

    const QColor m_batteryRed{235, 76, 92};
    const QColor m_batteryGreen{81, 196, 120};
    const QColor m_batteryYellow{240, 196, 65};

    const QColor m_rectColor{72, 31, 74};

    const QColor m_needleColor{192, 58, 75};
    const QColor m_needleOuterColor{Qt::white};

    const QColor m_commStatusColor{255, 38, 38};

    const QColor m_signalActiveGreen{0, 240, 20};
    const QColor m_signalInactiveGreen{
        m_signalActiveGreen.red(),
        m_signalActiveGreen.green(),
        m_signalActiveGreen.blue(),
        55, // Alpha
    };

    // Thresholds
    const int m_batteryCriticalThresh{25};
    const int m_batteryWarningThresh{50};

    const int m_tempLowThresh{5};
    const int m_tempMidThresh{40};

    // Radiis (Radius)
    const int m_needleOuterRadius{17};
    const int m_needleInnerRadius{10};

    // Dimensions
    const int m_windowWidth{600};
    const int m_windowHeight{420};

    const int m_defaultWindowWidth{840};
    const int m_defaultWindowHeight{588};

    const int m_minWindowWidth{472};
    const int m_minWindowHeight{334};

    const int m_commIconSize{28};
    const int m_commIconHeight{34};

    const int m_arrowIconSize{34};

    const int m_messageBoxHeight{22};

    const int m_needleBaseHalfWidth{5};

    const int m_gaugeArcPenWidth{6};

    const int m_tickMajorLength{22};
    const int m_tickMinorLength{10};
    const int m_tickMajorWidth{5};
    const int m_tickMinorWidth{3};

    const int m_labelRectWidth{52};
    const int m_labelRectHeight{28};

    const int m_speedIconRectWidth{56};
    const int m_speedIconRectHeight{44};

    const int m_unitRectWidth{164};
    const int m_unitRectHeight{34};

    const int m_commRectWidth{200};
    const int m_commRectHeight{70};

    const int m_arrowRectWidth{58};
    const int m_arrowRectHeight{52};

    const int m_batteryRectWidth{46};
    const int m_batteryRectHeight{86};
    const int m_batteryOuterPenWidth{6};
    const int m_batteryOuterRadius{6};

    const int m_batteryCapHeight{12};
    const int m_batteryCapWidthOffset{20};
    const int m_batteryCapRadius{3};

    const int m_batteryFillRadius{2};

    const int m_batteryTextWidthOffset{12};
    const int m_batteryTextHeight{26};

    const int m_tempIconRectHeight{56};

    const int m_tempTextWidthOffset{16};
    const int m_tempTextHeight{28};

    // Offsets
    const int m_needleTailOffset{4};
    const int m_needleLengthOffset{36};

    const int m_gaugeCenterXOffset{255};
    const int m_gaugeCenterYOffset{260};

    const int m_tickOuterRadiusOffset{7};
    const int m_labelRadiusOffset{58};

    const int m_labelRectXOffset{26};
    const int m_labelRectYOffset{14};

    const int m_speedIconXOffset{28};
    const int m_speedIconYOffset{45};

    const int m_unitXOffset{82};
    const int m_unitYOffset{88};

    const int m_commXOffset{100};
    const int m_commYOffset{58};

    const int m_rightArrowXOffset{76};
    const int m_leftArrowXOffset{18};
    const int m_arrowYOffset{28};

    const int m_batteryRectXOffset{92};
    const int m_batteryRectYOffset{150};

    const int m_batteryCapXOffset{10};
    const int m_batteryCapYOffset{10};

    const int m_batteryInnerOffset{7};

    const int m_batteryTextXOffset{6};
    const int m_batteryTextYOffset{2};

    const int m_tempIconYOffset{104};

    const int m_tempTextXOffset{8};
    const int m_tempTextYOffset{48};

    // Font families
    QString m_iconFontFamily{"Material Icons"};
    QString m_defaultFontFamily{"Sans Serif"};

    // Font file names
    QString m_iconFontFamilyFileName{"MaterialIcons.ttf"};

    // Font sizes
    const int m_statusFontSize{13};

    const int m_unitFontSize{22};
    const int m_speedIconFontSize{32};
    const int m_speedLabelFontSize{17};

    const int m_batteryFontSize{14};
    const int m_tempIconFontSize{42};

    // Messages
    const QString m_commErrorMsg{"Connection Error"};

    // Signals
    int m_speed{0};
    int m_temperature{0};
    int m_batteryLevel{0};
    bool m_leftLight{false};
    bool m_rightLight{false};

    // Intervals
    const int m_minorTickInterval{5};
    const int m_majorTickInterval{20};

    // Multipliers
    const int m_qtAngleMultiplier{16};

    // Angles
    const qreal m_gaugeStartAngle{225.0};
    const qreal m_gaugeSweepAngle{270.0};

    // Sounds
    QString m_signalSoundCommand;
    QProcess m_signalSoundProcess;
    QString m_leftSignalSoundPath;
    QString m_rightSignalSoundPath;
    QString m_warningSignalSoundPath;
    bool m_stoppingSignalSound{false};
    SignalSound m_activeSignalSound{SignalSound::None};

    // Sound file names
    QString m_leftSignalSoundFileName{"sound_left.wav"};
    QString m_rightSignalSoundFileName{"sound_right.wav"};
    QString m_warningSignalSoundFileName{"sound_warning.wav"};

    // Icons
    const ushort m_ErrorIcon{0xe628};
    const ushort m_SpeedIcon{0xe9e4};
    const ushort m_BatteryIcon{0xebdc};
    const ushort m_LeftArrowIcon{0xe5c4};
    const ushort m_RightArrowIcon{0xe5c8};
    const ushort m_TemperatureIcon{0xe1ff};

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
