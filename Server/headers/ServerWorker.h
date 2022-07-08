#ifndef SERVER_SERVERWORKER_H
#define SERVER_SERVERWORKER_H

#include <QObject>

class QTcpSocket;

class ServerWorker : public QObject {
Q_OBJECT

    Q_DISABLE_COPY(ServerWorker)

public:
    explicit ServerWorker(QObject *parent = nullptr);

    virtual bool setSocketDescriptor(qintptr socketDescriptor);

    QString userName() const;

    void setUserName(const QString &userName);

    void sendJson(const QJsonObject &jsonData);

signals:

    void jsonReceived(const QJsonObject &jsonDoc);

    void disconnectedFromClient();

    void error();

    void logMessage(const QString &msg);

public slots:

    void disconnectFromClient();

private slots:

    void receiveJson();

private:
    QTcpSocket *socket;
    QString m_userName;
};

#endif //SERVER_SERVERWORKER_H
