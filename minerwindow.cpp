#include "minerwindow.h"

MinerWindow::MinerWindow() {
    qsrand(QTime::currentTime().msec());

    init();
    trainNetwork();

    startTimer(100);
}

MinerWindow::~MinerWindow() {
    delete net;
}

void MinerWindow::timerEvent(QTimerEvent *) {
    takeScreenshot();
    recognize();
    update();
}

void MinerWindow::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Escape:
        close();
        break;
    }
}

void MinerWindow::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        SendMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
        SendMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
    } else if (e->button() == Qt::RightButton) {
        SendMessage(hWnd, WM_RBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
        SendMessage(hWnd, WM_RBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
    }
}

void MinerWindow::mouseDoubleClickEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
        SendMessage(hWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(0, 0));
}

void MinerWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);

    for (int i = 0; i < fieldWidth; i++)
        for (int j = 0; j < fieldHeight; j++)
            p.drawImage(QPointF(i * iconWidth, j * iconHeight), icons[map[i][j]].scaled(iconWidth, iconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void MinerWindow::init() {
    hWnd = FindWindow(L"Minesweeper", 0);

    icons.reserve(11);

    for (int i = 0; i < 11; i++)
        icons << QImage("data/" + QString::number(i * 3 + 1) + ".bmp");

    map.resize(fieldWidth);

    for (int i = 0; i < map.size(); i++) {
        map[i].resize(fieldHeight);
        map[i].fill(0);
    }
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
    std::vector<std::vector<double>> data;

    for (uint i = 0; i < 36; i++)
        data.push_back(processImage(QImage("data/" + QString::number(i) + ".bmp").scaled(trainWidth, trainHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));

    std::vector<Network::Example> examples;

    for (uint i = 0; i < data.size(); i++)
        examples.push_back(Network::Example(data[i], i < 33 ? i / 3 : 10));

    net = new Network({trainWidth * trainHeight * 3, 24, 11});

    net->setLearningRate(0.01);
    net->setMomentum(0.1);
    net->setL2Decay(0);
    net->setMaxLoss(1e-2);
    net->setMaxEpochs(1000);
    net->setBatchSize(1);
    net->setVerbose(false);

    net->train(examples);
}

void MinerWindow::takeScreenshot() {
    field = QApplication::primaryScreen()->grabWindow((WId)hWnd).toImage();

    if (field.isNull())
        return;

    int left = field.width() * 0.05;
    int top = field.height() * 0.085;
    int w = field.width() * 0.95 - left;
    int h = field.height() * 0.905 - top;

    field = field.copy(left, top, w, h);

    iconWidth = (double)field.width() / fieldWidth;
    iconHeight = (double)field.height() / fieldHeight;

    resize(field.size());
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

            const static int threshold = 90;

            while (true) {
                QRgb rgb = field.pixel(x, yc);
                if (qRed(rgb) <= threshold && qGreen(rgb) <= threshold && qBlue(rgb) <= threshold)
                    break;
                x--;
            }

            while (true) {
                QRgb rgb = field.pixel(xc, y);
                if (qRed(rgb) <= threshold && qGreen(rgb) <= threshold && qBlue(rgb) <= threshold)
                    break;
                y--;
            }

            QImage icon = field.copy(x + 1, y + 1, iconWidth - 2, iconHeight - 2).scaled(trainWidth, trainHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            map[i][j] = net->predict(processImage(icon));
        }
}
