#include "TransferProtocol.h"
#include <QJsonDocument>
#include <QJsonObject>

void TransferProtocol::sendMessage(QTcpSocket *socket, const QString &msg, const QVariant &from, const QVariant &to) {
    QJsonObject recordObject;
    recordObject.insert("msg", msg);
    recordObject.insert("from", from.toString());
    recordObject.insert("type", TransferProtocol::NewMessage);
    recordObject.insert("to", to.toString());

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendNewGuyInfo(QTcpSocket *socket, const QJsonArray &desc, const QJsonArray &names) {
    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    if (!names.isEmpty()) {
        recordObject.insert("name", names);
    }
    recordObject.insert("type", TransferProtocol::NewUsers);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendDeadGuyInfo(QTcpSocket *socket, const QJsonArray &desc) {
    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    recordObject.insert("type", TransferProtocol::DelUsers);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendSocketDescriptor(QTcpSocket *socket) {
    QJsonObject recordObject;
    recordObject.insert("descriptor", QString::number(socket->socketDescriptor()));
    recordObject.insert("type", TransferProtocol::YourSocketDescriptor);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendUserName(QTcpSocket *socket, const QVariant &from, const QString &newName) {
    QJsonObject recordObject;
    recordObject.insert("type", TransferProtocol::NewUserName);
    recordObject.insert("from", from.toString());
    recordObject.insert("name", newName);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendWithHeader(QTcpSocket *socket, const QJsonObject &obj) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QByteArray block;
    QDataStream stream(&block, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_3);
    stream << qint32(0) << obj;
    if (stream.device()->size() > std::numeric_limits<qint32>::max() + sizeof(qint32)) {
        return;
    }
    stream.device()->seek(0);
    stream << qint32(block.size() - sizeof(qint32));
    socket->write(block);
}
