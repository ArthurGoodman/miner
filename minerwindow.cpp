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

void MinerWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);

    for (int i = 0; i < fieldWidth; i++)
        for (int j = 0; j < fieldHeight; j++)
            p.drawImage(QPointF(i * iconWidth, j * iconHeight), icons[map[i][j]].scaled(iconWidth, iconHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void MinerWindow::init() {
    icons.reserve(11);

    for (uint i = 0; i < 11; i++)
        icons.push_back(QImage("data/" + QString::number(i < 9 ? i : i == 9 ? 10 : 13) + ".bmp"));

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

    for (uint i = 0; i < 15; i++)
        data.push_back(processImage(QImage("data/" + QString::number(i) + ".bmp")));

    std::vector<Network::Example> examples;

    for (uint i = 0; i < data.size(); i++)
        examples.push_back(Network::Example(data[i], i < 9 ? i : i < 12 ? 9 : 10));

    net = new Network({48 * 48 * 3, 11});

    net->setLearningRate(0.01);
    net->setMomentum(0.1);
    net->setL2Decay(0.001);
    net->setMaxLoss(1e-4);
    net->setMaxEpochs(1000);
    net->setBatchSize(1);
    // net->setVerbose(false);

    net->train(examples);
}

void MinerWindow::takeScreenshot() {
    field = QApplication::primaryScreen()->grabWindow((WId)FindWindow(L"Minesweeper", 0)).toImage();

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
    for (int i = 0; i < fieldWidth; i++)
        for (int j = 0; j < fieldHeight; j++) {
            QImage icon = field.copy(i * iconWidth + 1, j * iconHeight + 1, iconWidth, iconHeight);
            icon = icon.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            map[i][j] = net->predict(processImage(icon));
        }
}
