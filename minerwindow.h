#pragma once

#include <QtWidgets>

#include "network.h"

class MinerWindow : public QWidget {
    const int fieldWidth = 30;
    const int fieldHeight = 16;

    const int trainWidth = 20;
    const int trainHeight = 20;

    QImage field;
    QVector<QImage> icons;
    Network *net;

    double iconWidth, iconHeight;

    QVector<QVector<int>> map;

    HWND hWnd;

public:
    MinerWindow();
    ~MinerWindow();

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    void init();
    std::vector<double> processImage(const QImage &image);
    void trainNetwork();
    void takeScreenshot();
    void recognize();
};
