#include <QtMath>
#include "canvas.h"
#include "setting.h"
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QPen>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

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

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    const int fontId = QFontDatabase::addApplicationFont("desktop/client/res/MaterialIcons.ttf");
    if (fontId != -1)
    {
        m_iconFontFamily = QFontDatabase::applicationFontFamilies(fontId).value(0, m_iconFontFamily);
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString resDir = QDir::cleanPath(appDir + "/../desktop/client/res");
    m_leftSignalSoundPath = QDir::cleanPath(appDir + "/sound_left.wav");
    m_rightSignalSoundPath = QDir::cleanPath(appDir + "/sound_right.wav");
    m_warningSignalSoundPath = QDir::cleanPath(appDir + "/sound_warning.wav");
    if (!QFileInfo::exists(m_leftSignalSoundPath))
    {
        m_leftSignalSoundPath = QDir::cleanPath(resDir + "/sound_left.wav");
    }
    if (!QFileInfo::exists(m_rightSignalSoundPath))
    {
        m_rightSignalSoundPath = QDir::cleanPath(resDir + "/sound_right.wav");
    }
    if (!QFileInfo::exists(m_warningSignalSoundPath))
    {
        m_warningSignalSoundPath = QDir::cleanPath(resDir + "/sound_warning.wav");
    }
    m_leftSignalSoundPath = QFileInfo(m_leftSignalSoundPath).absoluteFilePath();
    m_rightSignalSoundPath = QFileInfo(m_rightSignalSoundPath).absoluteFilePath();
    m_warningSignalSoundPath = QFileInfo(m_warningSignalSoundPath).absoluteFilePath();
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
    setMinimumSize(472, 334);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

Canvas::~Canvas()
{
    stopSignalSound();
}

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

void Canvas::setLightSignals(bool leftLight, bool rightLight, bool warningLight)
{
    m_leftLight = leftLight;
    m_rightLight = rightLight;
    m_warningLight = warningLight;

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

void Canvas::setCommunicationStatus(bool connected, const QString &message)
{
    m_connected = connected;
    m_communicationMessage = message;

    if (!m_connected)
    {
        m_speed = 0;
        m_temperature = 0;
        m_batteryLevel = 0;
        m_leftLight = false;
        m_rightLight = false;
        m_warningLight = false;
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

void Canvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(72, 31, 74));

    const QSizeF designSize(600, 420);
    const qreal scale = qMin(width() / designSize.width(), height() / designSize.height());
    const QPointF offset((width() - designSize.width() * scale) / 2.0,
                         (height() - designSize.height() * scale) / 2.0);

    painter.translate(offset);
    painter.scale(scale, scale);
    painter.fillRect(QRectF(QPointF(0, 0), designSize), QColor(72, 31, 74));

    const QRectF panel(QPointF(0, 0), designSize);
    drawGauge(painter, QRectF(0, 0, 600, 420));
    drawSideIndicators(painter, panel);
}

QColor Canvas::temperatureColor() const
{
    if (m_temperature < 5)
    {
        return QColor(245, 247, 250);
    }
    if (m_temperature <= 39)
    {
        return QColor(48, 150, 255);
    }
    return QColor(235, 76, 92);
}

QColor Canvas::batteryColor() const
{
    if (m_batteryLevel < 25)
    {
        return QColor(235, 76, 92);
    }
    if (m_batteryLevel <= 49)
    {
        return QColor(240, 196, 65);
    }
    return QColor(81, 196, 120);
}

bool Canvas::hasActiveLightSignal() const
{
    return m_leftLight || m_rightLight || m_warningLight;
}

Canvas::SignalSound Canvas::selectedSignalSound() const
{
    // Decide which sound should be used.
    // Warning means both arrows, so it uses the normal stereo sound.
    // Left and right use separate sound files.
    if (m_warningLight || (m_leftLight && m_rightLight))
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

void Canvas::stopSignalSound()
{
    m_stoppingSignalSound = true;
    if (m_signalSoundProcess.state() != QProcess::NotRunning)
    {
        m_signalSoundProcess.kill();
        m_signalSoundProcess.waitForFinished(100);
    }
    m_stoppingSignalSound = false;
}

void Canvas::startSignalSound()
{
    if (m_signalSoundCommand.isEmpty())
    {
        return;
    }
    if (m_signalSoundProcess.state() == QProcess::NotRunning)
    {
        const QString soundPath = soundPathFor(m_activeSignalSound);
        if (!soundPath.isEmpty())
        {
            m_signalSoundProcess.start(m_signalSoundCommand, {soundPath});
        }
    }
}

QPointF Canvas::pointOnGauge(const QPointF &center, qreal radius, int speed) const
{
    // Convert the speed value to a position on the round speedometer.
    // First we convert speed to an angle, then we use sin/cos to get x and y.
    const qreal startAngle = 225.0;
    const qreal sweepAngle = 270.0;
    const auto &info = speedInfo();
    const qreal speedRange = info.max - info.min;
    const qreal speedRatio = speedRange == 0 ? 0.0 : (speed - info.min) / speedRange;
    const qreal angle = qDegreesToRadians(startAngle - (sweepAngle * speedRatio));
    return QPointF(center.x() + qCos(angle) * radius,
                   center.y() - qSin(angle) * radius);
}

void Canvas::drawGauge(QPainter &painter, const QRectF &rect) const
{
    const qreal radius = 225;
    const QPointF center(rect.left() + 255, rect.top() + 260);

    // Draw the big white arc around the speedometer.
    // The numbers and ticks will be placed around this arc.
    painter.setPen(QPen(Qt::white, 6));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(QRectF(center.x() - radius, center.y() - radius,
                           radius * 2, radius * 2),
                    225 * 16, -270 * 16);

    const auto &speedLimit = speedInfo();
    for (int value = speedLimit.min; value <= speedLimit.max; value += 5)
    {
        // Draw the small and big tick marks.
        // Small ticks are every 5 km/h. Big ticks and numbers are every 20 km/h.
        const bool majorTick = value % 20 == 0;
        const qreal outerRadius = radius - 7;
        const qreal innerRadius = outerRadius - (majorTick ? 22 : 10);
        const QPointF outer = pointOnGauge(center, outerRadius, value);
        const QPointF inner = pointOnGauge(center, innerRadius, value);

        painter.setPen(QPen(Qt::white, majorTick ? 5 : 3));
        painter.drawLine(inner, outer);

        if (majorTick)
        {
            const QPointF labelPoint = pointOnGauge(center, radius - 58, value);
            QRectF labelRect(labelPoint.x() - 26, labelPoint.y() - 14, 52, 28);
            QFont labelFont = painter.font();
            labelFont.setPointSize(17);
            labelFont.setBold(false);
            painter.setFont(labelFont);
            painter.setPen(Qt::white);
            painter.drawText(labelRect, Qt::AlignCenter, QString::number(value));
        }
    }

    drawNeedle(painter, center, radius);

    if (m_connected)
    {
        painter.setPen(Qt::white);
        QFont iconFont(m_iconFontFamily);
        iconFont.setPointSize(32);
        painter.setFont(iconFont);
        painter.drawText(QRectF(center.x() - 28, center.y() + 45, 56, 44),
                         Qt::AlignCenter, QString(QChar(m_SpeedIcon)));

        QFont unitFont = painter.font();
        unitFont.setFamily("Sans Serif");
        unitFont.setPointSize(22);
        unitFont.setBold(true);
        painter.setFont(unitFont);
        painter.drawText(QRectF(center.x() - 82, center.y() + 88, 164, 34),
                         Qt::AlignCenter, QString("%1 km/h").arg(m_speed));
    }
    else
    {
        drawCommunication(painter, QRectF(center.x() - 100, center.y() + 58, 200, 70));
    }
}

void Canvas::drawNeedle(QPainter &painter, const QPointF &center, qreal radius) const
{
    // Draw the red needle.
    // The needle tip is placed on the gauge using the current speed value.
    const QPointF tip = pointOnGauge(center, radius - 36, m_speed);
    const qreal angle = qAtan2(center.y() - tip.y(), tip.x() - center.x());
    const QPointF normal(-qSin(angle), -qCos(angle));
    const QPointF base(center.x() - qCos(angle) * 4, center.y() + qSin(angle) * 4);

    QPolygonF needle;
    needle << tip
           << QPointF(base.x() + normal.x() * 5, base.y() + normal.y() * 5)
           << QPointF(base.x() - normal.x() * 5, base.y() - normal.y() * 5);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(192, 58, 75));
    painter.drawPolygon(needle);

    painter.setBrush(Qt::white);
    painter.drawEllipse(center, 17, 17);
    painter.setBrush(QColor(192, 58, 75));
    painter.drawEllipse(center, 10, 10);
}

void Canvas::drawSideIndicators(QPainter &painter, const QRectF &rect) const
{
    // Draw left and right arrows.
    // They are weak green when inactive and bright green when they blink.
    const bool showLeft = m_blinkVisible && (m_leftLight || m_warningLight);
    const bool showRight = m_blinkVisible && (m_rightLight || m_warningLight);
    const QColor activeGreen(0, 240, 20);
    const QColor inactiveGreen(0, 240, 20, 55);

    QFont iconFont(m_iconFontFamily);
    iconFont.setPointSize(34);
    painter.setFont(iconFont);

    painter.setPen(showRight ? activeGreen : inactiveGreen);
    painter.drawText(QRectF(rect.right() - 76, rect.top() + 28, 58, 52),
                     Qt::AlignCenter, QString(QChar(m_RightArrowIcon)));

    painter.setPen(showLeft ? activeGreen : inactiveGreen);
    painter.drawText(QRectF(rect.left() + 18, rect.top() + 28, 58, 52),
                     Qt::AlignCenter, QString(QChar(m_LeftArrowIcon)));

    const QColor battery = batteryColor();
    const QRectF batteryRect(rect.right() - 92, rect.top() + 150, 46, 86);
    // Draw the battery cap and outer shape.
    // The color depends on the current battery level.
    painter.setPen(Qt::NoPen);
    painter.setBrush(battery);
    painter.drawRoundedRect(QRectF(batteryRect.left() + 10, batteryRect.top() - 10,
                                   batteryRect.width() - 20, 12),
                            3, 3);

    painter.setPen(QPen(battery, 6));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(batteryRect, 6, 6);

    const QRectF batteryInner = batteryRect.adjusted(7, 7, -7, -7);
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
    painter.drawRoundedRect(batteryFillRect, 2, 2);

    QFont textFont = painter.font();
    textFont.setFamily("Sans Serif");
    textFont.setPointSize(14);
    textFont.setBold(true);
    painter.setFont(textFont);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(batteryRect.left() - 6, batteryRect.bottom() + 2,
                            batteryRect.width() + 12, 26),
                     Qt::AlignCenter, QString("%1%").arg(m_batteryLevel));

    QFont tempIconFont(m_iconFontFamily);
    tempIconFont.setPointSize(42);
    painter.setFont(tempIconFont);
    painter.setPen(temperatureColor());
    painter.drawText(QRectF(batteryRect.left(), rect.bottom() - 104,
                            batteryRect.width(), 56),
                     Qt::AlignCenter, QString(QChar(m_TemperatureIcon)));

    painter.setFont(textFont);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(batteryRect.left() - 8, rect.bottom() - 48,
                            batteryRect.width() + 16, 28),
                     Qt::AlignCenter, QString("%1 \u00b0C").arg(m_temperature));
}

void Canvas::drawCommunication(QPainter &painter, const QRectF &rect) const
{
    const QColor statusColor(255, 38, 38);
    const QString statusIcon{QChar(m_ErrorIcon)};

    QFont iconFont(m_iconFontFamily);
    iconFont.setPointSize(28);
    painter.setFont(iconFont);
    painter.setPen(statusColor);
    painter.drawText(QRectF(rect.left(), rect.top(), rect.width(), 34),
                     Qt::AlignCenter, statusIcon);

    painter.setPen(statusColor);
    QFont statusFont = painter.font();
    statusFont.setFamily("Sans Serif");
    statusFont.setPointSize(13);
    statusFont.setBold(true);
    painter.setFont(statusFont);
    painter.drawText(QRectF(rect.left(), rect.top() + 34, rect.width(), 22),
                     Qt::AlignCenter, "Connection Error");
}
