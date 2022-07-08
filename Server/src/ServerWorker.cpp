#include "ServerWorker.h"
#include <QtNetwork/QTcpSocket>
#include <QJsonParseError>
#include <QJsonObject>

ServerWorker::ServerWorker(QObject *parent) : QObject(parent), socket(new QTcpSocket(this)) {
    connect(socket, &QTcpSocket::readyRead, this, &ServerWorker::receiveJson);
    connect(socket, &QTcpSocket::disconnected, this, &ServerWorker::disconnectedFromClient);
    connect(socket, &QAbstractSocket::errorOccurred, this, &ServerWorker::error);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor) {
    return socket->setSocketDescriptor(socketDescriptor);
}

void ServerWorker::disconnectFromClient() {
    socket->disconnectFromHost();
}

QString ServerWorker::userName() const {
    return m_userName;
}

void ServerWorker::setUserName(const QString &userName) {
    m_userName = userName;
}

void ServerWorker::receiveJson() {
    QByteArray jsonData;
    QDataStream stream(socket);
    stream.setVersion(QDataStream::Qt_6_3);
    forever {
        stream.startTransaction();
        stream >> jsonData;
        if (stream.commitTransaction()) {
            QJsonParseError error;
            const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
            if (error.error == QJsonParseError::NoError) {
                if (doc.isObject()) {
                    emit jsonReceived(doc.object());
                } else {
                    emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
                }
            } else {
                emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            }
        } else {
            break;
        }
    }
}

void ServerWorker::sendJson(const QJsonObject &json) {
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage(tr("Sending to %1 - %2").arg(userName(), jsonData));
    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_6_3);
    socketStream << jsonData;
}