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
    int wndX, wndY, clientWidth, clientHeight;

    enum Cell {
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
        Seven,
        Eight,
        Flag,
        Bomb,
        Open,
        Closed
    };

    bool run;

public:
    MinerWindow();
    ~MinerWindow();

protected:
    void timerEvent(QTimerEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);

private:
    void init();
    void start();
    std::vector<double> processImage(const QImage &image);
    void trainNetwork();
    void takeScreenshot();
    void cropField();
    void recognize();
    void process();
    void leftClick(int x, int y);
    void rightClick(int x, int y);
    void doubleClick(int x, int y);
    void moveCursor(int x, int y);
    int sumOfNeighbors(int x, int y, Cell cell);
    bool isNearDigit(int x, int y);
    void placeFlags(int x, int y);
};
