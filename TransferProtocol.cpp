#include "TransferProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QJsonObject TransferProtocol::sendMessage(const QString &msg, const QVariant &from, const QVariant &to) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::NewMessage);
    recordObject.insert("msg", msg);
    recordObject.insert("from", from.toString());
    recordObject.insert("to", to.toString());

    return recordObject;
}

QJsonObject TransferProtocol::sendNewGuyInfo(const QJsonArray &names) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::NewUsers);
    recordObject.insert("names", names);

    return recordObject;
}

QJsonObject TransferProtocol::sendDeadGuyInfo(const QJsonArray &names) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::DelUsers);
    recordObject.insert("names", names);

    return recordObject;
}

QJsonObject TransferProtocol::sendUserName(const QString &newName) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::NewUserName);
    recordObject.insert("name", newName);

    return recordObject;
}

QJsonObject TransferProtocol::sendLoggedIn(const bool &logged, const QString &reason) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::LoggedIn);
    recordObject.insert("logged", logged);
    recordObject.insert("reason", reason);

    return recordObject;
}
