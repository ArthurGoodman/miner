#include "minerwindow.h"

MinerWindow::MinerWindow()
    : run(false) {
    qsrand(QTime::currentTime().msec());

    init();
    trainNetwork();

    startTimer(100);

    // start();
}

MinerWindow::~MinerWindow() {
    delete net;
}

void MinerWindow::timerEvent(QTimerEvent *) {
    // if (GetKeyState(VK_ESCAPE) & 0xff00) {
    //     qApp->quit();
    //     return;
    // }

    takeScreenshot();
    recognize();

    if (run)
        process();

    update();
}

void MinerWindow::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        close();
        break;

    case Qt::Key_Space:
        start();
        break;
    }
}

void MinerWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);

    for (int i = 0; i < fieldWidth; i++)
        for (int j = 0; j < fieldHeight; j++)
            p.drawImage(QPointF(i * iconWidth, j * iconHeight), icons[map[i][j]].scaled(iconWidth, iconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void MinerWindow::init() {
    hWnd = FindWindow(L"Minesweeper", 0);

    RECT r;
    GetClientRect(hWnd, &r);

    wndX = r.left;
    wndY = r.top;

    map.resize(fieldWidth);

    for (int i = 0; i < map.size(); i++) {
        map[i].resize(fieldHeight);
        map[i].fill(0);
    }

    icons.resize(12);
}

void MinerWindow::start() {
    SetForegroundWindow(hWnd);
    run = true;
}

std::vector<double> MinerWindow::processImage(const QImage &image) {
    std::vector<double> result;

    for (int x = 0; x < image.width(); x++)
        for (int y = 0; y < image.height(); y++) {
            QRgb color = image.pixel(x, y);

            result.push_back((double)qRed(color) / 255);
            result.push_back((double)qGreen(color) / 255);
            result.push_back((double)qBlue(color) / 255);
        }

    return result;
}

void MinerWindow::trainNetwork() {
    if (!QFile::exists("network")) {
        qDebug() << "Training network...";

        net = new Network({trainWidth * trainHeight * 3, 24, 12});

        net->setLearningRate(0.01);
        net->setMomentum(0.1);
        net->setL2Decay(0);
        net->setMaxLoss(1e-2);
        net->setMaxEpochs(1000);
        net->setBatchSize(1);
        net->setVerbose(false);

        std::vector<Network::Example> examples;

        QFile file(":/data/data.txt");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);

        QString line;
        while (stream.readLineInto(&line)) {
            if (line.isEmpty())
                continue;

            QTextStream stream(&line);

            int label;
            stream >> label;

            bool first = true;

            while (true) {
                QString fileName;
                stream >> fileName;

                if (fileName.isNull())
                    break;

                QImage image(":/data/" + fileName + ".bmp");

                if (first) {
                    first = false;
                    icons[label] = image;
                }

                examples.push_back(Network::Example(processImage(image.scaled(trainWidth, trainHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)), label));
            }
        }

        net->train(examples);

        net->saveToFile("network");
    } else {
        QFile file(":/data/data.txt");
        file.open(QFile::ReadOnly | QFile::Text);
        QTextStream stream(&file);

        QString line;
        while (stream.readLineInto(&line)) {
            if (line.isEmpty())
                continue;

            QTextStream stream(&line);

            int label;
            stream >> label;

            QString fileName;
            stream >> fileName;

            QImage image(":/data/" + fileName + ".bmp");

            icons[label] = image;
        }

        net = new Network;
        *net = Network::loadFromFile("network");
    }
}

void MinerWindow::takeScreenshot() {
    field = QApplication::primaryScreen()->grabWindow((WId)hWnd).toImage();

    if (field.isNull())
        return;

    clientWidth = field.width();
    clientHeight = field.height();

    cropField();

    resize(field.size());
}

void MinerWindow::cropField() {
    int left = field.width() * 0.05;
    int top = field.height() * 0.085;
    int w = field.width() * 0.95 - left;
    int h = field.height() * 0.905 - top;

    field = field.copy(left, top, w, h);

    iconWidth = (double)field.width() / fieldWidth;
    iconHeight = (double)field.height() / fieldHeight;
}

void MinerWindow::recognize() {
    if (field.isNull())
        return;

    for (int i = 0; i < fieldWidth; i++)
        for (int j = 0; j < fieldHeight; j++) {
            int xc = i * iconWidth + iconWidth / 2;
            int yc = j * iconHeight + iconHeight / 2;

            int x = xc;
            int y = yc;

            const static int threshold = 75;

            while (x > 0) {
                QRgb rgb = field.pixel(x, yc);
                if (qRed(rgb) <= threshold && qGreen(rgb) <= threshold && qBlue(rgb) <= threshold)
                    break;
                x--;
            }

            while (y > 0) {
                QRgb rgb = field.pixel(xc, y);
                if (qRed(rgb) <= threshold && qGreen(rgb) <= threshold && qBlue(rgb) <= threshold)
                    break;
                y--;
            }

            // int x = xc;
            // QRgb rgb = field.pixel(xc, yc);
            // int minSum = qRed(rgb) + qGreen(rgb) + qBlue(rgb);

            // for (int dx = -1; dx >= -iconWidth * 0.75 && dx >= -xc; dx--) {
            //  QRgb rgb = field.pixel(xc + dx, yc);
            //  int sum = qRed(rgb) + qGreen(rgb) + qBlue(rgb);

            //  if (sum < minSum) {
            //      x = xc + dx;
            //      minSum = sum;
            //  }
            // }

            // int y = yc;
            // rgb = field.pixel(xc, yc);
            // minSum = qRed(rgb) + qGreen(rgb) + qBlue(rgb);`

            // for (int dy = -1; dy >= -iconHeight * 0.75 && dy >= -yc; dy--) {
            //  QRgb rgb = field.pixel(xc, yc + dy);
            //  int sum = qRed(rgb) + qGreen(rgb) + qBlue(rgb);

            //  if (sum < minSum) {
            //      y = yc + dy;
            //      minSum = sum;
            //  }
            // }

            QImage icon = field.copy(x + 1, y + 1, iconWidth - 2, iconHeight - 2).scaled(trainWidth, trainHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            map[i][j] = net->predict(processImage(icon));
        }
}

void MinerWindow::process() {
    if (field.isNull())
        return;

    QList<QPoint> points;
    QVector<QPoint> closed, closedNearDigits;

    bool allClosed = true, allOpen = true;

    for (int x = 0; x < fieldWidth; x++)
        for (int y = 0; y < fieldHeight; y++) {
            if (map[x][y] == Bomb) {
                qDebug() << "Fail.";
                run = false;
                return;
            }

            if (map[x][y] == Closed) {
                if (isNearDigit(x, y))
                    closedNearDigits << QPoint(x, y);
                else
                    closed << QPoint(x, y);

                allOpen = false;
            } else
                allClosed = false;

            if (map[x][y] >= One && map[x][y] <= Eight) {
                int s = sumOfNeighbors(x, y, Closed);
                if (s > 0 && s + sumOfNeighbors(x, y, Flag) == map[x][y] + 1)
                    points << QPoint(x, y);
            }
        }

    if (allOpen) {
        qDebug() << "Success!";
        run = false;
        return;
    }

    if (allClosed) {
        leftClick(qrand() % fieldWidth, qrand() % fieldHeight);
        return;
    }

    foreach (const QPoint &p, points)
        placeFlags(p.x(), p.y());

    bool opened = false;

    for (int x = 0; x < fieldWidth; x++)
        for (int y = 0; y < fieldHeight; y++)
            if (map[x][y] >= One && map[x][y] <= Eight) {
                int s = sumOfNeighbors(x, y, Closed);
                if (s > 0 && sumOfNeighbors(x, y, Flag) == map[x][y] + 1) {
                    opened = true;
                    doubleClick(x, y);
                }
            }

    if (points.isEmpty() && !opened) {
        // for (int x = 0; x < fieldWidth; x++)
        //     for (int y = 0; y < fieldHeight; y++)
        //         if (sumOfNeighbors(x, y, Closed) > 0)
        //             points << QPoint(x, y);

        // foreach (const QPoint &p, points) {
        //     int f = sumOfNeighbors(p.x(), p.y(), Flag);

        //     for (int dx = -1; dx <= 1; dx++)
        //         for (int dy = -1; dy <= 1; dy++) {
        //             if (dx == 0 && dy == 0)
        //                 continue;

        //             QPoint d(p.x() + dx, p.y() + dy);

        //             if (map[d.x()][d.y()] >= One && map[d.x()][d.y()] <= Eight) {
        //                 int cd = 0;
        //                 int fd = 0;

        //                 for (int dx = -1; dx <= 1; dx++)
        //                     for (int dy = -1; dy <= 1; dy++) {
        //                         if ((dx == 0 && dy == 0) || abs(p.x() - d.x() - dx) <= 1 || abs(p.y() - d.y() - dy) <= 1)
        //                             continue;

        //                         if (map[d.x() + dx][d.y() + dy] == Closed)
        //                             cd++;
        //                         else if (map[d.x() + dx][d.y() + dy] == Flag)
        //                             fd++;
        //                     }

        //                 if(map[p.x()][p.y()] + 1 - f + fd)
        //             }
        //         }
        // }

        qDebug() << "Random guess...";
        QPoint p = closedNearDigits.isEmpty() ? closed[qrand() % closed.size()] : closedNearDigits[qrand() % closedNearDigits.size()];
        leftClick(p.x(), p.y());
    }
}

void MinerWindow::leftClick(int x, int y) {
    moveCursor(x, y);
    SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
    SendMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}

void MinerWindow::rightClick(int x, int y) {
    moveCursor(x, y);
    SendMessage(hWnd, WM_RBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
    SendMessage(hWnd, WM_RBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}

void MinerWindow::doubleClick(int x, int y) {
    moveCursor(x, y);
    SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
    SendMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
    SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
    SendMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
}

void MinerWindow::moveCursor(int x, int y) {
    POINT p = {(int)(clientWidth * 0.05 + x * iconWidth + iconWidth / 2), (int)(clientHeight * 0.085 + y * iconHeight + iconHeight / 2)};
    ClientToScreen(hWnd, &p);
    SetCursorPos(p.x, p.y);

    Sleep(10);
}

int MinerWindow::sumOfNeighbors(int x, int y, Cell cell) {
    int sum = 0;

    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0)
                continue;

            if (x + dx >= 0 && x + dx < fieldWidth && y + dy >= 0 && y + dy < fieldHeight)
                if (map[x + dx][y + dy] == cell)
                    sum++;
        }

    return sum;
}

bool MinerWindow::isNearDigit(int x, int y) {
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0)
                continue;

            if (x + dx >= 0 && x + dx < fieldWidth && y + dy >= 0 && y + dy < fieldHeight)
                if (map[x + dx][y + dy] >= One && map[x + dx][y + dy] <= Eight)
                    return true;
        }

    return false;
}

void MinerWindow::placeFlags(int x, int y) {
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0)
                continue;

            if (x + dx >= 0 && x + dx < fieldWidth && y + dy >= 0 && y + dy < fieldHeight)
                if (map[x + dx][y + dy] == Closed) {
                    map[x + dx][y + dy] = Flag;
                    rightClick(x + dx, y + dy);
                }
        }
}
