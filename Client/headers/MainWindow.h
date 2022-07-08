#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <QWidget>
#include <QAbstractSocket>

class QTextBrowser;

class QShortcut;

class QTcpSocket;

class VerticalTabWidget;

class ClientConnection;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

    enum SHOW_MESSAGE_TYPE {
        FromServerMessage,
        SentMessage,
        NormalMessage
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private slots:

    void attemptConnection();

    void connectedToServer();

    void attemptLogin(const QString &name);

    void loggedIn();

    void loginFailed(const QString &reason);

    void messageReceived(const QString &msg, const QString &from, const QString &to);

    void sendMessage();

    void disconnectedFromServer();

    void newUsers(const QJsonArray &names);

    void delUsers(const QJsonArray &names);

    void error(QAbstractSocket::SocketError socketError);

    void showMessage(const QString &msg, MainWindow::SHOW_MESSAGE_TYPE messageType, int ind);

signals:

    void newMessage(const QString &msg, MainWindow::SHOW_MESSAGE_TYPE messageType, int ind);

private:
    Ui::MainWindow *ui;

    QVector<QString> otherSockets;
    QVector<QTextBrowser *> textBrowsers;
    QShortcut *keyCtrlTab;
    const QString allChat = "All";
    const QString serverMessage = "Server";

    ClientConnection *connection;

    void createTabWidget(const QString &title);

    void prepareTabWidget();
};


#endif //CLIENT_MAINWINDOW_H
