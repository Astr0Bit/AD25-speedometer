#ifndef CANVAS_H
#define CANVAS_H

#include <QDir>
#include <QColor>
#include <QWidget>
#include <QPainter>
#include <QProcess>
#include <QFileInfo>
#include <QPaintEvent>
#include <QStandardPaths>
#include <QCoreApplication>

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
    static constexpr int m_waitTime_ms{100};

    // Colors
    static constexpr QColor m_tempRed{235, 76, 92};
    static constexpr QColor m_tempBlue{48, 150, 255};
    static constexpr QColor m_tempWhite{245, 247, 250};

    static constexpr QColor m_batteryRed{235, 76, 92};
    static constexpr QColor m_batteryGreen{81, 196, 120};
    static constexpr QColor m_batteryYellow{240, 196, 65};

    static constexpr QColor m_rectColor{72, 31, 74};

    static constexpr QColor m_needleColor{192, 58, 75};
    static inline const QColor m_needleOuterColor{Qt::white};

    static constexpr QColor m_commStatusColor{255, 38, 38};

    static constexpr QColor m_signalActiveGreen{0, 240, 20};
    static inline const QColor m_signalInactiveGreen{
        m_signalActiveGreen.red(),
        m_signalActiveGreen.green(),
        m_signalActiveGreen.blue(),
        55, // Alpha
    };

    // Thresholds
    static constexpr int m_batteryCriticalThresh{25};
    static constexpr int m_batteryWarningThresh{50};

    static constexpr int m_tempLowThresh{5};
    static constexpr int m_tempMidThresh{40};

    // Radiis (Radius)
    static constexpr int m_needleOuterRadius{17};
    static constexpr int m_needleInnerRadius{10};

    // Dimensions
    static constexpr int m_windowWidth{600};
    static constexpr int m_windowHeight{420};

    static constexpr int m_defaultWindowWidth{840};
    static constexpr int m_defaultWindowHeight{588};

    static constexpr int m_minWindowWidth{472};
    static constexpr int m_minWindowHeight{334};

    static constexpr int m_commIconSize{28};
    static constexpr int m_commIconHeight{34};

    static constexpr int m_arrowIconSize{34};

    static constexpr int m_messageBoxHeight{22};

    static constexpr int m_needleBaseHalfWidth{5};

    static constexpr int m_gaugeArcPenWidth{6};

    static constexpr int m_tickMajorLength{22};
    static constexpr int m_tickMinorLength{10};
    static constexpr int m_tickMajorWidth{5};
    static constexpr int m_tickMinorWidth{3};

    static constexpr int m_labelRectWidth{52};
    static constexpr int m_labelRectHeight{28};

    static constexpr int m_speedIconRectWidth{56};
    static constexpr int m_speedIconRectHeight{44};

    static constexpr int m_unitRectWidth{164};
    static constexpr int m_unitRectHeight{34};

    static constexpr int m_commRectWidth{200};
    static constexpr int m_commRectHeight{70};

    static constexpr int m_arrowRectWidth{58};
    static constexpr int m_arrowRectHeight{52};

    static constexpr int m_batteryRectWidth{46};
    static constexpr int m_batteryRectHeight{86};
    static constexpr int m_batteryOuterPenWidth{6};
    static constexpr int m_batteryOuterRadius{6};

    static constexpr int m_batteryCapHeight{12};
    static constexpr int m_batteryCapWidthOffset{20};
    static constexpr int m_batteryCapRadius{3};

    static constexpr int m_batteryFillRadius{2};

    static constexpr int m_batteryTextWidthOffset{12};
    static constexpr int m_batteryTextHeight{26};

    static constexpr int m_tempIconRectHeight{56};

    static constexpr int m_tempTextWidthOffset{16};
    static constexpr int m_tempTextHeight{28};

    // Offsets
    static constexpr int m_needleTailOffset{4};
    static constexpr int m_needleLengthOffset{36};

    static constexpr int m_gaugeCenterXOffset{255};
    static constexpr int m_gaugeCenterYOffset{260};

    static constexpr int m_tickOuterRadiusOffset{7};
    static constexpr int m_labelRadiusOffset{58};

    static constexpr int m_labelRectXOffset{26};
    static constexpr int m_labelRectYOffset{14};

    static constexpr int m_speedIconXOffset{28};
    static constexpr int m_speedIconYOffset{45};

    static constexpr int m_unitXOffset{82};
    static constexpr int m_unitYOffset{88};

    static constexpr int m_commXOffset{100};
    static constexpr int m_commYOffset{58};

    static constexpr int m_rightArrowXOffset{76};
    static constexpr int m_leftArrowXOffset{18};
    static constexpr int m_arrowYOffset{28};

    static constexpr int m_batteryRectXOffset{92};
    static constexpr int m_batteryRectYOffset{150};

    static constexpr int m_batteryCapXOffset{10};
    static constexpr int m_batteryCapYOffset{10};

    static constexpr int m_batteryInnerOffset{7};

    static constexpr int m_batteryTextXOffset{6};
    static constexpr int m_batteryTextYOffset{2};

    static constexpr int m_tempIconYOffset{104};

    static constexpr int m_tempTextXOffset{8};
    static constexpr int m_tempTextYOffset{48};

    // Font families
    QString m_iconFontFamily{"Material Icons"};
    QString m_defaultFontFamily{"Sans Serif"};

    // Font file names
    const QString m_iconFontFamilyFileName{"MaterialIcons.ttf"};

    // Font sizes
    static constexpr int m_statusFontSize{13};

    static constexpr int m_unitFontSize{22};
    static constexpr int m_speedIconFontSize{32};
    static constexpr int m_speedLabelFontSize{17};

    static constexpr int m_batteryFontSize{14};
    static constexpr int m_tempIconFontSize{42};

    // Messages
    const QString m_commErrorMsg{"Connection Error"};

    // Signals
    int m_speed{0};
    int m_temperature{0};
    int m_batteryLevel{0};
    bool m_leftLight{false};
    bool m_rightLight{false};

    // Intervals
    static constexpr int m_minorTickInterval{5};
    static constexpr int m_majorTickInterval{20};

    // Multipliers
    static constexpr int m_qtAngleMultiplier{16};

    // Angles
    static constexpr qreal m_gaugeStartAngle{225.0};
    static constexpr qreal m_gaugeSweepAngle{270.0};

    // Sound file names
    // * NOTE: These must be initialized before the paths
    const QString m_leftSignalSoundFileName{"sound_left.wav"};
    const QString m_rightSignalSoundFileName{"sound_right.wav"};
    const QString m_warningSignalSoundFileName{"sound_warning.wav"};

    // Directories
    // * NOTE: These must be initialized before the paths
    const QString m_appDir = QCoreApplication::applicationDirPath() + "/";
    const QString m_resDir = QDir::cleanPath(m_appDir + "../desktop/client/res") + "/";

    // Sounds
    QString m_signalSoundCommand;
    QProcess m_signalSoundProcess;
    const QString m_leftSignalSoundPath;
    const QString m_rightSignalSoundPath;
    const QString m_warningSignalSoundPath;
    bool m_stoppingSignalSound{false};
    SignalSound m_activeSignalSound{SignalSound::None};

    // Icons
    static constexpr ushort m_ErrorIcon{0xe628};
    static constexpr ushort m_SpeedIcon{0xe9e4};
    static constexpr ushort m_BatteryIcon{0xebdc};
    static constexpr ushort m_LeftArrowIcon{0xe5c4};
    static constexpr ushort m_RightArrowIcon{0xe5c8};
    static constexpr ushort m_TemperatureIcon{0xe1ff};

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
