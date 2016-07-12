#include <QApplication>

#include "minerwindow.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    MinerWindow w;
    w.show();

    return app.exec();
}
