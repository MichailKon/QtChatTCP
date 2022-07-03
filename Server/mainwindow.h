#ifndef SERVER_MAINWINDOW_H
#define SERVER_MAINWINDOW_H

#include <QWidget>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class mainwindow; }
QT_END_NAMESPACE

class mainwindow : public QWidget {
Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);

    ~mainwindow() override;

signals:

    void newMessage(QString &msg);

private slots:

    void newConnection();

    void readSocket();

    void discardSocket();

    void appendToSockets(QTcpSocket *socket);

    void showMessage(const QString &msg);

    void sendMessageButtonClicked();

private:
    Ui::mainwindow *ui;

    QTcpServer *server;
    QMap<int, QTcpSocket *> desc2con;
    QMap<QTcpSocket *, int> con2desc;

    const QString allChat = "Whole chat";
};


#endif //SERVER_MAINWINDOW_H
