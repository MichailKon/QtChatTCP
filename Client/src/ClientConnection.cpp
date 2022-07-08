#include "ClientConnection.h"
#include "TransferProtocol.h"
#include <QTcpSocket>
#include <QJsonParseError>
#include <QJsonObject>

ClientConnection::ClientConnection(QObject *parent) : QObject(parent), m_socket(new QTcpSocket), m_loggedIn(false) {
    connect(m_socket, &QTcpSocket::connected, this, &ClientConnection::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientConnection::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientConnection::readyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &ClientConnection::error);
    connect(m_socket, &QTcpSocket::disconnected, this, [this]()->void{m_loggedIn = false;});
}

void ClientConnection::login(const QString &userName) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QDataStream stream(m_socket);
        stream.setVersion(QDataStream::Qt_6_3);
        QJsonObject obj = TransferProtocol::sendUserName(userName);
        stream << QJsonDocument(obj).toJson();
    }
}

void ClientConnection::sendMessage(const QString &msg, const QString &to) {
    if (msg.isEmpty()) {
        return;
    }
    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_6_3);
    QJsonObject obj = TransferProtocol::sendMessage(msg, "", to);
    stream << QJsonDocument(obj).toJson();
}

void ClientConnection::disconnect() {
    m_socket->disconnectFromHost();
}

void ClientConnection::connectToServer(const QHostAddress &address, quint16 port) {
    m_socket->connectToHost(address, port);
}

void ClientConnection::jsonReceived(const QJsonObject &data) {
    if (!data.contains("type")) {
        return;
    }
    TransferProtocol::DATA_TYPES type = static_cast<TransferProtocol::DATA_TYPES>(data["type"].toInt());
    if (type == TransferProtocol::NewUsers) {
        if (!data.contains("names")) {
            return;
        }
        emit newUsers(data["names"].toArray());
    } else if (type == TransferProtocol::DelUsers) {
        if (!data.contains("names")) {
            return;
        }
        emit delUsers(data["names"].toArray());
    } else if (type == TransferProtocol::NewMessage) {
        if (!data.contains("msg") || !data.contains("from") || !data.contains("to")) {
            return;
        }
        emit newMessage(data["msg"].toString(), data["from"].toString(), data["to"].toString());
    } else if (type == TransferProtocol::LoggedIn) {
        if (m_loggedIn) {
            return;
        }
        if (!data.contains("logged") || !data.contains("reason")) {
            return;
        }
        bool logged = data["logged"].toBool();
        if (logged) {
            m_loggedIn = true;
            emit loggedIn();
            return;
        }
        QString reason = data["reason"].toString();
        emit loginError(reason);
    } else {
        Q_UNREACHABLE();
    }
}

void ClientConnection::readyRead() {
    QByteArray jsonData;
    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_6_3);
    forever {
        stream.startTransaction();
        stream >> jsonData;
        if (stream.commitTransaction()) {
            QJsonParseError error;
            const QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
            if (error.error == QJsonParseError::NoError) {
                if (doc.isObject()) {
                    jsonReceived(doc.object());
                }
            }
        } else {
            break;
        }
    }
}

bool ClientConnection::isLoggedIn() const {
    return m_loggedIn;
}
