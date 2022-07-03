#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    auto *window = new mainwindow();
    window->show();
    return QApplication::exec();
}
