#ifndef TRANSFER_PROTOCOL_H
#define TRANSFER_PROTOCOL_H

#include <QString>
#include <QJsonArray>

namespace TransferProtocol {
    const QString SERVER = "Server";
    const QString ALL = "AllChat";

    enum DATA_TYPES {
        LoggedIn,
        NewMessage,
        NewUsers,
        DelUsers,
        NewUserName
    };

    QJsonObject sendMessage(const QString &msg, const QVariant &from, const QVariant &to);

    QJsonObject sendNewGuyInfo(const QJsonArray &names);

    QJsonObject sendDeadGuyInfo(const QJsonArray &names);

    QJsonObject sendUserName(const QString &newName);

    QJsonObject sendLoggedIn(const bool &logged, const QString &reason);
}

#endif