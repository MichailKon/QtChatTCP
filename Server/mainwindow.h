#ifndef SERVER_MAINWINDOW_H
#define SERVER_MAINWINDOW_H

#include <QWidget>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui { class mainwindow; }
QT_END_NAMESPACE

class mainwindow : public QWidget {
Q_OBJECT

    enum SHOW_MESSAGE_TYPES {
        InfoMessage,
        SentMessage,
        NormalMessage
    };

public:
    explicit mainwindow(QWidget *parent = nullptr);

    ~mainwindow() override;

signals:

    void newMessage(QString &msg, mainwindow::SHOW_MESSAGE_TYPES from);

    void newData(QTcpSocket *socket, const QJsonDocument &doc);

private slots:

    void newConnection();

    void readSocket();

    void discardSocket();

    void appendToSockets(QTcpSocket *socket);

    void showMessage(QString msg, SHOW_MESSAGE_TYPES from);

    void sendMessageButtonClicked();

    void handleData(QTcpSocket *socket, const QJsonDocument &doc);

private:
    Ui::mainwindow *ui;

    QTcpServer *server;
    QMap<int, QTcpSocket *> desc2con;
    QMap<QTcpSocket *, int> con2desc;
    QShortcut *keyEnter;

    const QString allChat = "Whole chat";
    qint32 nextBlock = 0;
};


#endif //SERVER_MAINWINDOW_H
