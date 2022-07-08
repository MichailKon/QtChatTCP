#ifndef CLIENT_CLIENTCONNECTION_H
#define CLIENT_CLIENTCONNECTION_H

#include <QObject>
#include <QAbstractSocket>

class QHostAddress;

class QTcpSocket;

class ClientConnection : public QObject {
Q_OBJECT

    Q_DISABLE_COPY(ClientConnection)

public:
    explicit ClientConnection(QObject *parent = nullptr);

    bool isLoggedIn() const;

public slots:

    void connectToServer(const QHostAddress &address, quint16 port);

    void sendMessage(const QString &msg, const QString &to);

    void login(const QString &userName);

    void disconnect();

private slots:

    void readyRead();

signals:

    void connected();

    void loggedIn();

    void loginError(const QString &reason);

    void disconnected();

    void error(QAbstractSocket::SocketError socketError);

    void newUsers(const QJsonArray &names);

    void delUsers(const QJsonArray &names);

    void newMessage(const QString &msg, const QString &from, const QString &to);

private:
    QTcpSocket *m_socket;
    bool m_loggedIn;

    void jsonReceived(const QJsonObject &data);
};

#endif //CLIENT_CLIENTCONNECTION_H
