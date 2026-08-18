#ifndef WINDOW_H
#define WINDOW_H

#include <QTimer>
#include <QDialog>
#include "canvas.h"
#include <QGridLayout>
#include "comservice.h"

class Window : public QDialog
{
public:
    explicit Window(COMService &com_service, QWidget *parent = nullptr);

private:
    Canvas m_canvas;
    QTimer m_blinkTimer;
    QGridLayout m_layout;
    COMService &m_com_service;

    // Periodically check connection status, and read buffer
    QTimer m_connBufTimer;
};

#endif
