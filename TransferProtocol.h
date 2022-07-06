#ifndef TRANSFER_PROTOCOL_H
#define TRANSFER_PROTOCOL_H

#include <QtNetwork/QTcpSocket>
#include <QString>
#include <QJsonArray>

namespace TransferProtocol {
    const QString SERVER = "Server";
    const QString ALL = "AllChat";

    enum DATA_TYPES {
        NewMessage,
        NewUsers,
        DelUsers,
        YourSocketDescriptor,
        NewUserName
    };

    enum VERSIONS {
        TP_1_0_0
    };

    void sendMessage(QTcpSocket *socket, const QString &msg, const QVariant &from, const QVariant &to);

    void sendNewGuyInfo(QTcpSocket *socket, const QJsonArray &desc, const QJsonArray &names = {});

    void sendDeadGuyInfo(QTcpSocket *socket, const QJsonArray &desc);

    void sendSocketDescriptor(QTcpSocket *socket);

    void sendUserName(QTcpSocket *socket, const QVariant &from, const QString &newName);

    void sendWithHeader(QTcpSocket *socket, const QJsonObject &obj);
}

#endif