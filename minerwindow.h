#pragma once

#include <QtWidgets>

#include "network.h"

class MinerWindow : public QWidget {
    const int fieldWidth = 30;
    const int fieldHeight = 16;

    QImage field;
    QVector<QImage> icons;
    Network *net;

    double iconWidth, iconHeight;

    QVector<QVector<int>> map;

public:
    MinerWindow();
    ~MinerWindow();

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    void init();
    std::vector<double> processImage(const QImage &image);
    void trainNetwork();
    void takeScreenshot();
    void recognize();
};
