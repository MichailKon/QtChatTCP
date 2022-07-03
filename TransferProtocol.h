#ifndef TRANSFER_PROTOCOL_H
#define CLIENT_MAINWINDOW_H

#include <QtNetwork/QTcpSocket>
#include <QString>

namespace TransferProtocol {
    const QString SERVER = "Server";

    enum DATA_TYPES {
        NewMessage,
        NewUsers,
        DelUsers,
        YourSocketDescriptor
    };

    enum VERSIONS {
        TP_1_0_0
    };

    void sendMessage(QTcpSocket *socket, const QString &msg, const QVariant &from);

    void sendNewGuyInfo(QTcpSocket *socket, const QJsonArray &desc);

    void sendDeadGuyInfo(QTcpSocket *socket, const QJsonArray &desc);

    void sendSocketDescriptor(QTcpSocket *socket);
}

#endif