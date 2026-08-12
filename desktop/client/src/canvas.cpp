#include <QPen>
#include <QDir>
#include <QFont>
#include <QtMath>
#include "canvas.h"
#include "setting.h"
#include <QFileInfo>
#include <QFontDatabase>
#include <QStandardPaths>
#include <QCoreApplication>
#include <iostream>

// Helper functions for getting Info structs for the required signals
const auto &speedInfo()
{
    return Setting::Signal::handle()["speed"];
}

const auto &temperatureInfo()
{
    return Setting::Signal::handle()["temperature"];
}

const auto &batteryInfo()
{
    return Setting::Signal::handle()["battery_level"];
}

// * === Canvas class === *
// Constructor
Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    // Construct relative paths to find the audio files and fonts
    const QString appDir = QCoreApplication::applicationDirPath() + "/";
    const QString resDir = QDir::cleanPath(appDir + "../desktop/client/res") + "/";

    // Find the icon font
    const int fontId = QFontDatabase::addApplicationFont(resDir + m_iconFontFamilyFileName);
    if (fontId != -1)
    {
        m_iconFontFamily = QFontDatabase::applicationFontFamilies(fontId).value(0, m_iconFontFamily);
    }

    // Get relative paths for the respective audio files
    m_leftSignalSoundPath = QDir::cleanPath(resDir + m_leftSignalSoundFileName);
    m_rightSignalSoundPath = QDir::cleanPath(resDir + m_rightSignalSoundFileName);
    m_warningSignalSoundPath = QDir::cleanPath(resDir + m_warningSignalSoundFileName);

    // Get absolute paths for the respective audio files
    m_leftSignalSoundPath = QFileInfo(m_leftSignalSoundPath).absoluteFilePath();
    m_rightSignalSoundPath = QFileInfo(m_rightSignalSoundPath).absoluteFilePath();
    m_warningSignalSoundPath = QFileInfo(m_warningSignalSoundPath).absoluteFilePath();

    // Attach a lambda method to the audio command
    m_signalSoundCommand = QStandardPaths::findExecutable("pw-play");
    if (m_signalSoundCommand.isEmpty())
    {
        m_signalSoundCommand = QStandardPaths::findExecutable("aplay");
    }
    connect(&m_signalSoundProcess, &QProcess::finished, this,
            [this](int, QProcess::ExitStatus)
            {
                if (!m_stoppingSignalSound && hasActiveLightSignal())
                {
                    startSignalSound();
                }
            });

    // Set minimum window size
    setMinimumSize(m_minWindowWidth, m_minWindowHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

// Destructor
Canvas::~Canvas()
{
    stopSignalSound();
}

// Events
// Paints the canvas on an update() event
void Canvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill the GUI background
    painter.fillRect(rect(), m_rectColor);

    // Set the GUI size
    const QSizeF designSize(m_windowWidth, m_windowHeight);

    // Scale if resized
    const qreal scale = qMin(width() / designSize.width(), height() / designSize.height());
    const QPointF offset((width() - designSize.width() * scale) / 2.0,
                         (height() - designSize.height() * scale) / 2.0);
    painter.translate(offset);
    painter.scale(scale, scale);
    painter.fillRect(QRectF(QPointF(0, 0), designSize), m_rectColor);
    const QRectF panel(QPointF(0, 0), designSize);

    // Draw gauge and indicators
    drawGauge(painter, QRectF(0, 0, m_windowWidth, m_windowHeight));
    drawSideIndicators(painter, panel);
}

// To set the default window size
QSize Canvas::sizeHint() const
{
    return QSize(m_defaultWindowWidth, m_defaultWindowHeight);
}

// Setter methods
void Canvas::setSpeed(int speed)
{
    const auto &info = speedInfo();
    m_speed = qBound(info.min, speed, info.max);
    update();
}

void Canvas::setTemperature(int temperature)
{
    const auto &info = temperatureInfo();
    m_temperature = qBound(info.min, temperature, info.max);
    update();
}

void Canvas::setBatteryLevel(int batteryLevel)
{
    const auto &info = batteryInfo();
    m_batteryLevel = qBound(info.min, batteryLevel, info.max);
    update();
}

void Canvas::setLightSignals(bool leftLight, bool rightLight)
{
    m_leftLight = leftLight;
    m_rightLight = rightLight;

    // This checks which light signal is active now.
    // If it changed, we stop the old sound and start the correct new sound.
    const SignalSound newSound = selectedSignalSound();
    if (newSound == SignalSound::None)
    {
        m_activeSignalSound = SignalSound::None;
        stopSignalSound();
    }
    else if (newSound != m_activeSignalSound)
    {
        stopSignalSound();
        m_activeSignalSound = newSound;
        startSignalSound();
    }
    else
    {
        startSignalSound();
    }
    update();
}

void Canvas::setCommunicationStatus(bool connected)
{
    m_connected = connected;

    if (!m_connected)
    {
        m_speed = 0;
        m_temperature = 0;
        m_batteryLevel = 0;
        m_leftLight = false;
        m_rightLight = false;
        m_activeSignalSound = SignalSound::None;
        stopSignalSound();
    }

    update();
}

void Canvas::setBlinkVisible(bool visible)
{
    m_blinkVisible = visible;
    update();
}

// Sound methods
void Canvas::stopSignalSound()
{
    // Stop the sound
    m_stoppingSignalSound = true;

    // Stop the audio process if it's running
    if (m_signalSoundProcess.state() != QProcess::NotRunning)
    {
        m_signalSoundProcess.kill();
        m_signalSoundProcess.waitForFinished(m_waitTime_ms);
    }

    // Reset so other sounds can be played
    m_stoppingSignalSound = false;
}

void Canvas::startSignalSound()
{
    // Don't do anything if no command
    if (m_signalSoundCommand.isEmpty())
    {
        return;
    }

    // Start the audio
    if (m_signalSoundProcess.state() == QProcess::NotRunning)
    {
        const QString soundPath = soundPathFor(m_activeSignalSound);
        if (!soundPath.isEmpty())
        {
            m_signalSoundProcess.start(m_signalSoundCommand, {soundPath});
        }
    }
}

Canvas::SignalSound Canvas::selectedSignalSound() const
{
    // Decide which sound should be used.
    // Warning means both arrows, so it uses the normal stereo sound.
    // Left and right use separate sound files.
    if (m_leftLight && m_rightLight)
    {
        return SignalSound::Warning;
    }
    if (m_leftLight)
    {
        return SignalSound::Left;
    }
    if (m_rightLight)
    {
        return SignalSound::Right;
    }
    return SignalSound::None;
}

// Color methods
QColor Canvas::temperatureColor() const
{
    if (m_temperature < m_tempLowThresh)
    {
        return m_tempWhite;
    }
    if (m_temperature < m_tempMidThresh)
    {
        return m_tempBlue;
    }
    return m_tempRed;
}

QColor Canvas::batteryColor() const
{
    if (m_batteryLevel < m_batteryCriticalThresh)
    {
        return m_batteryRed;
    }
    if (m_batteryLevel < m_batteryWarningThresh)
    {
        return m_batteryYellow;
    }
    return m_batteryGreen;
}

// Draw methods
QPointF Canvas::pointOnGauge(const QPointF &center, qreal radius, int speed) const
{
    // Convert the speed value to a position on the round speedometer.
    // First we convert speed to an angle, then we use sin/cos to get x and y.
    const auto &info = speedInfo();
    const qreal speedRange = info.max - info.min;
    const qreal speedRatio = speedRange == 0 ? 0.0 : (speed - info.min) / speedRange;
    const qreal angle = qDegreesToRadians(m_gaugeStartAngle - (m_gaugeSweepAngle * speedRatio));
    return QPointF(center.x() + qCos(angle) * radius,
                   center.y() - qSin(angle) * radius);
}

void Canvas::drawGauge(QPainter &painter, const QRectF &rect) const
{
    // Determine radius and center for where to put the gauge
    const qreal radius = m_gaugeStartAngle;
    const QPointF center(
        rect.left() + m_gaugeCenterXOffset,
        rect.top() + m_gaugeCenterYOffset);

    // Draw the big white arc around the speedometer.
    // The numbers and ticks will be placed around this arc.
    painter.setPen(QPen(Qt::white, m_gaugeArcPenWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(
        QRectF(center.x() - radius,
               center.y() - radius,
               radius * 2, radius * 2),
        m_gaugeStartAngle * m_qtAngleMultiplier,
        -m_gaugeSweepAngle * m_qtAngleMultiplier);

    // Get speed limit from Info struct
    const auto &speedLimit = speedInfo();
    for (int value = speedLimit.min; value <= speedLimit.max; value += m_minorTickInterval)
    {
        // Draw the small and big tick marks.
        const bool majorTick = value % m_majorTickInterval == 0;
        const qreal outerRadius = radius - m_tickOuterRadiusOffset;
        const qreal innerRadius = outerRadius - (majorTick ? m_tickMajorLength : m_tickMinorLength);
        const QPointF outer = pointOnGauge(center, outerRadius, value);
        const QPointF inner = pointOnGauge(center, innerRadius, value);

        painter.setPen(QPen(Qt::white, majorTick ? m_tickMajorWidth : m_tickMinorWidth));
        painter.drawLine(inner, outer);

        // Put speed value on the bigger tick marks
        if (majorTick)
        {
            const QPointF labelPoint = pointOnGauge(center, radius - m_labelRadiusOffset, value);
            QRectF labelRect(
                labelPoint.x() - m_labelRectXOffset,
                labelPoint.y() - m_labelRectYOffset,
                m_labelRectWidth, m_labelRectHeight);
            QFont labelFont = painter.font();
            labelFont.setPointSize(m_speedLabelFontSize);
            labelFont.setBold(false);
            painter.setFont(labelFont);
            painter.setPen(Qt::white);
            painter.drawText(labelRect, Qt::AlignCenter, QString::number(value));
        }
    }

    drawNeedle(painter, center, radius);

    // Put connected icon, and current speed on gauge
    if (m_connected)
    {
        painter.setPen(Qt::white);
        QFont iconFont(m_iconFontFamily);
        iconFont.setPointSize(m_speedIconFontSize);
        painter.setFont(iconFont);
        painter.drawText(
            QRectF(center.x() - m_speedIconXOffset,
                   center.y() + m_speedIconYOffset,
                   m_speedIconRectWidth, m_speedIconRectHeight),
            Qt::AlignCenter, QString(QChar(m_SpeedIcon)));

        QFont unitFont = painter.font();
        unitFont.setFamily(m_defaultFontFamily);
        unitFont.setPointSize(m_unitFontSize);
        unitFont.setBold(true);
        painter.setFont(unitFont);
        painter.drawText(
            QRectF(center.x() - m_unitXOffset,
                   center.y() + m_unitYOffset,
                   m_unitRectWidth, m_unitRectHeight),
            Qt::AlignCenter, QString("%1 km/h").arg(m_speed));
    }

    // Put disconnected icon, and "Connection error" message on gauge
    else
    {
        drawCommunication(
            painter,
            QRectF(center.x() - m_commXOffset,
                   center.y() + m_commYOffset,
                   m_commRectWidth, m_commRectHeight));
    }
}

void Canvas::drawNeedle(QPainter &painter, const QPointF &center, qreal radius) const
{
    // Calculate the tip position
    const QPointF tip = pointOnGauge(center, radius - m_needleLengthOffset, m_speed);

    // Calculate needle angle
    const qreal angle = qAtan2(center.y() - tip.y(), tip.x() - center.x());
    const QPointF normal(-qSin(angle), -qCos(angle));
    const QPointF base(
        center.x() - qCos(angle) * m_needleTailOffset,
        center.y() + qSin(angle) * m_needleTailOffset);

    // Create the needle as a polygon
    QPolygonF needle;
    needle << tip
           << QPointF(base.x() + normal.x() * m_needleBaseHalfWidth,
                      base.y() + normal.y() * m_needleBaseHalfWidth)
           << QPointF(base.x() - normal.x() * m_needleBaseHalfWidth,
                      base.y() - normal.y() * m_needleBaseHalfWidth);

    // Draw the needle
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_needleColor);
    painter.drawPolygon(needle);

    // Draw the outer center circle
    painter.setBrush(m_needleOuterColor);
    painter.drawEllipse(center, m_needleOuterRadius, m_needleOuterRadius);

    // Draw the inner center circle
    painter.setBrush(m_needleColor);
    painter.drawEllipse(center, m_needleInnerRadius, m_needleInnerRadius);
}

void Canvas::drawSideIndicators(QPainter &painter, const QRectF &rect) const
{
    // Draw left and right arrows.
    // They are weak green when inactive and bright green when they blink.
    const bool showLeft = m_blinkVisible && m_leftLight;
    const bool showRight = m_blinkVisible && m_rightLight;

    QFont iconFont(m_iconFontFamily);
    iconFont.setPointSize(m_arrowIconSize);
    painter.setFont(iconFont);

    painter.setPen(showRight ? m_signalActiveGreen : m_signalInactiveGreen);
    painter.drawText(QRectF(rect.right() - m_rightArrowXOffset,
                            rect.top() + m_arrowYOffset,
                            m_arrowRectWidth, m_arrowRectHeight),
                     Qt::AlignCenter, QString(QChar(m_RightArrowIcon)));

    painter.setPen(showLeft ? m_signalActiveGreen : m_signalInactiveGreen);
    painter.drawText(QRectF(rect.left() + m_leftArrowXOffset,
                            rect.top() + m_arrowYOffset,
                            m_arrowRectWidth, m_arrowRectHeight),
                     Qt::AlignCenter, QString(QChar(m_LeftArrowIcon)));

    const QColor battery = batteryColor();
    const QRectF batteryRect(rect.right() - m_batteryRectXOffset,
                             rect.top() + m_batteryRectYOffset,
                             m_batteryRectWidth, m_batteryRectHeight);

    // Draw the battery cap and outer shape.
    // The color depends on the current battery level.
    painter.setPen(Qt::NoPen);
    painter.setBrush(battery);
    painter.drawRoundedRect(QRectF(batteryRect.left() + m_batteryCapXOffset,
                                   batteryRect.top() - m_batteryCapYOffset,
                                   batteryRect.width() - m_batteryCapWidthOffset,
                                   m_batteryCapHeight),
                            m_batteryCapRadius, m_batteryCapRadius);

    painter.setPen(QPen(battery, m_batteryOuterPenWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(batteryRect, m_batteryOuterRadius, m_batteryOuterRadius);

    const QRectF batteryInner = batteryRect.adjusted(m_batteryInnerOffset,
                                                     m_batteryInnerOffset,
                                                     -m_batteryInnerOffset,
                                                     -m_batteryInnerOffset);

    // Fill the battery from bottom to top.
    // A high percent fills almost the whole battery. A low percent fills only the bottom.
    const auto &batteryLimit = batteryInfo();
    const qreal batteryRange = batteryLimit.max - batteryLimit.min;
    const qreal batteryRatio = batteryRange == 0 ? 0.0 : (m_batteryLevel - batteryLimit.min) / batteryRange;
    const qreal batteryFill = qBound(0.0, batteryRatio, 1.0);
    const QRectF batteryFillRect(batteryInner.left(),
                                 batteryInner.top() + (batteryInner.height() * (1.0 - batteryFill)),
                                 batteryInner.width(),
                                 batteryInner.height() * batteryFill);
    painter.setPen(Qt::NoPen);
    painter.setBrush(battery);
    painter.drawRoundedRect(batteryFillRect, m_batteryFillRadius, m_batteryFillRadius);

    QFont textFont = painter.font();
    textFont.setFamily(m_defaultFontFamily);
    textFont.setPointSize(m_batteryFontSize);
    textFont.setBold(true);
    painter.setFont(textFont);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(batteryRect.left() - m_batteryTextXOffset,
                            batteryRect.bottom() + m_batteryTextYOffset,
                            batteryRect.width() + m_batteryTextWidthOffset,
                            m_batteryTextHeight),
                     Qt::AlignCenter, QString("%1%").arg(m_batteryLevel));

    QFont tempIconFont(m_iconFontFamily);
    tempIconFont.setPointSize(m_tempIconFontSize);
    painter.setFont(tempIconFont);
    painter.setPen(temperatureColor());
    painter.drawText(QRectF(batteryRect.left(),
                            rect.bottom() - m_tempIconYOffset,
                            batteryRect.width(),
                            m_tempIconRectHeight),
                     Qt::AlignCenter, QString(QChar(m_TemperatureIcon)));

    painter.setFont(textFont);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(batteryRect.left() - m_tempTextXOffset,
                            rect.bottom() - m_tempTextYOffset,
                            batteryRect.width() + m_tempTextWidthOffset,
                            m_tempTextHeight),
                     Qt::AlignCenter, QString("%1 \u00b0C").arg(m_temperature));
}
void Canvas::drawCommunication(QPainter &painter, const QRectF &rect) const
{
    const QString statusIcon{QChar(m_ErrorIcon)};

    QFont iconFont(m_iconFontFamily);
    iconFont.setPointSize(m_commIconSize);
    painter.setFont(iconFont);
    painter.setPen(m_commStatusColor);
    painter.drawText(
        QRectF(rect.left(),
               rect.top(),
               rect.width(),
               m_commIconHeight),
        Qt::AlignCenter, statusIcon);

    painter.setPen(m_commStatusColor);
    QFont statusFont = painter.font();
    statusFont.setFamily(m_defaultFontFamily);
    statusFont.setPointSize(m_statusFontSize);
    statusFont.setBold(true);
    painter.setFont(statusFont);
    painter.drawText(
        QRectF(rect.left(),
               rect.top() + m_commIconHeight,
               rect.width(), m_messageBoxHeight),
        Qt::AlignCenter, m_commErrorMsg);
}

// Misc methods
bool Canvas::hasActiveLightSignal() const
{
    return m_leftLight || m_rightLight;
}

QString Canvas::soundPathFor(SignalSound sound) const
{
    // Return the file path for the sound we want to play.
    // This keeps the sound choice in one place.
    switch (sound)
    {
    case SignalSound::Left:
        return m_leftSignalSoundPath;
    case SignalSound::Right:
        return m_rightSignalSoundPath;
    case SignalSound::Warning:
        return m_warningSignalSoundPath;
    case SignalSound::None:
        return {};
    }

    return {};
}