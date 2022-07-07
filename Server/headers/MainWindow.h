#ifndef SERVER_MAINWINDOW_H
#define SERVER_MAINWINDOW_H

#include <QWidget>

class QShortcut;

class QTcpServer;

class QTcpSocket;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

    enum SHOW_MESSAGE_TYPES {
        InfoMessage,
        SentMessage,
        NormalMessage
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

signals:

    void newMessage(QString &msg, MainWindow::SHOW_MESSAGE_TYPES from);

    void newData(QTcpSocket *socket, const QJsonDocument &doc);

private slots:

    void newConnection();

    void readSocket();

    void discardSocket();

    void appendToSockets(QTcpSocket *socket);

    void showMessage(QString msg, MainWindow::SHOW_MESSAGE_TYPES from);

    void handleData(QTcpSocket *socket, const QJsonDocument &doc);

private:
    Ui::MainWindow *ui;

    QTcpServer *server;
    QMap<int, QTcpSocket *> desc2con;
    QMap<QTcpSocket *, int> con2desc;
    QMap<QTcpSocket *, QString> con2name;

    const QString allChat = "Whole chat";
    qint32 nextBlock = 0;
};


#endif //SERVER_MAINWINDOW_H
