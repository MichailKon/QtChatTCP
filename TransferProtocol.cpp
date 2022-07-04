#include "TransferProtocol.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

void TransferProtocol::sendMessage(QTcpSocket *socket, const QString &msg, const QVariant &from, const QVariant &to) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("msg", msg);
    recordObject.insert("from", from.toString());
    recordObject.insert("type", TransferProtocol::NewMessage);
    recordObject.insert("to", to.toString());

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendNewGuyInfo(QTcpSocket *socket, const QJsonArray &desc) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    recordObject.insert("type", TransferProtocol::NewUsers);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendDeadGuyInfo(QTcpSocket *socket, const QJsonArray &desc) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("descriptor", desc);
    recordObject.insert("type", TransferProtocol::DelUsers);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendSocketDescriptor(QTcpSocket *socket) {
    if (!socket) return;
    if (!socket->isOpen()) return;

    QJsonObject recordObject;
    recordObject.insert("descriptor", (int) socket->socketDescriptor());
    recordObject.insert("type", TransferProtocol::YourSocketDescriptor);

    TransferProtocol::sendWithHeader(socket, recordObject);
}

void TransferProtocol::sendWithHeader(QTcpSocket *socket, const QJsonObject &obj) {
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
