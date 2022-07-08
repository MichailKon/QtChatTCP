#ifndef SERVER_MAINWINDOW_H
#define SERVER_MAINWINDOW_H

#include <QWidget>

class ChatTcpServer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void logMessage(const QString &message);

    void buttonClicked();

private:
    Ui::MainWindow *ui;
    ChatTcpServer *server;

    const QString turnOnText = "Включить", turnOffText = "Выключить";

    void turnServerOn();

    void turnServerOff();
};


#endif //SERVER_MAINWINDOW_H
