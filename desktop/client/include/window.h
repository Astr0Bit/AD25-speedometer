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
    Canvas *canvas_{nullptr};
    QTimer blinkTimer_;
};

#endif
