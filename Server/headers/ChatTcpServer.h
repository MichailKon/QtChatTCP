#ifndef SERVER_CHATTCPSERVER_H
#define SERVER_CHATTCPSERVER_H

#include <QtNetwork/QTcpServer>
#include "ServerWorker.h"

class ChatTcpServer : public QTcpServer {
Q_OBJECT

    Q_DISABLE_COPY(ChatTcpServer)

public:
    explicit ChatTcpServer(QWidget *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:

    void logMessage(const QString &msg);

public slots:

    void stopServer();

private slots:

    void jsonReceived(ServerWorker *sender, const QJsonObject &doc);

    void userDisconnected(ServerWorker *sender);

    void userError(ServerWorker *sender);

private:
    QVector<ServerWorker *> clients;

    const QString allChat = "Whole chat";

    static void sendJson(ServerWorker *destination, const QJsonObject &message);
};


#endif //SERVER_CHATTCPSERVER_H
