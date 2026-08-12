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
    explicit Window(QWidget *parent = nullptr);

private:
    Canvas m_canvas;
    QGridLayout m_layout;
    QTimer m_blinkTimer;
};

#endif
