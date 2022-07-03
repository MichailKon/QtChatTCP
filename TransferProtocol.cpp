#include "TransferProtocol.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TransferProtocol::sendMessage(QTcpSocket *socket, const QString &msg, const QVariant &from) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("msg", msg);
    recordObject.insert("from", from.toString());
    recordObject.insert("type", TransferProtocol::NewMessage);
    QJsonDocument message(recordObject);

    QDataStream stream(socket);
    stream << message;
}

void TransferProtocol::sendNewGuyInfo(QTcpSocket *socket, const QJsonArray &desc) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    qDebug() << desc;
    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    recordObject.insert("type", TransferProtocol::NewUsers);
    QJsonDocument message(recordObject);

    QDataStream stream(socket);
    stream << message;
}

void TransferProtocol::sendDeadGuyInfo(QTcpSocket *socket, const QJsonArray &desc) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    recordObject.insert("type", TransferProtocol::DelUsers);
    QJsonDocument message(recordObject);

    QDataStream stream(socket);
    stream << message;
}

void TransferProtocol::sendSocketDescriptor(QTcpSocket *socket) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("descriptor", (int) socket->socketDescriptor());
    recordObject.insert("type", TransferProtocol::YourSocketDescriptor);
    QJsonDocument message(recordObject);

    QDataStream stream(socket);
    stream << message;
}
