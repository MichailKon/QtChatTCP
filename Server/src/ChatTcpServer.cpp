#include "ChatTcpServer.h"
#include "TransferProtocol.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QtNetwork/QTcpSocket>


ChatTcpServer::ChatTcpServer(QWidget *parent) : QTcpServer(parent) {}

void ChatTcpServer::incomingConnection(qintptr socketDescriptor) {
    auto *worker = new ServerWorker(this);
    if (!worker->setSocketDescriptor(socketDescriptor)) {
        worker->deleteLater();
        return;
    }
    connect(worker, &ServerWorker::disconnectedFromClient, this, [this, worker] { userDisconnected(worker); });
    connect(worker, &ServerWorker::error, this, [this, worker] { userError(worker); });
    connect(worker, &ServerWorker::jsonReceived, this,
            [this, worker](auto &&PH1) { jsonReceived(worker, std::forward<decltype(PH1)>(PH1)); });
    connect(worker, &ServerWorker::logMessage, this, &ChatTcpServer::logMessage);

    QJsonArray names;
    for (auto &i: clients) {
        names.append(i->userName());
    }
    sendJson(worker, TransferProtocol::sendNewGuyInfo(names));

    clients.push_back(worker);

    emit logMessage(QStringLiteral("New client connected"));
}

void ChatTcpServer::sendJson(ServerWorker *destination, const QJsonObject &message) {
    Q_ASSERT(destination);
    destination->sendJson(message);
}

void ChatTcpServer::jsonReceived(ServerWorker *sender, const QJsonObject &data) {
    if (!data.contains("type")) {
        sendJson(sender, TransferProtocol::sendLoggedIn(false, "Wrong data format"));
        return;
    }
    TransferProtocol::DATA_TYPES type = static_cast<TransferProtocol::DATA_TYPES>(data["type"].toInt());
    if (type == TransferProtocol::NewUserName) {
        if (!data.contains("name")) {
            sendJson(sender, TransferProtocol::sendLoggedIn(false, "Wrong data format"));
            return;
        }
        if (!sender->userName().isEmpty()) {
            sendJson(sender, TransferProtocol::sendLoggedIn(false, "You already have a name"));
            return;
        }
        QString name = data["name"].toString();
        for (auto &i: clients) {
            if (i->userName() == name) {
                sendJson(sender, TransferProtocol::sendLoggedIn(false, "Such a name already exists"));
                return;
            }
        }
        sendJson(sender, TransferProtocol::sendLoggedIn(true, ""));
        sender->setUserName(name);
        for (auto &i: clients) {
            if (i != sender) {
                sendJson(i, TransferProtocol::sendNewGuyInfo({name}));
            }
        }
        emit logMessage(tr("INFO :: %1 connected").arg(sender->userName()));
    } else if (type == TransferProtocol::NewMessage) {
        if (!data.contains("msg") || !data.contains("from") || !data.contains("to")) {
            return;
        }
        QString msg = data["msg"].toString(), to = data["to"].toString();
        if (to != TransferProtocol::SERVER && to != TransferProtocol::ALL) {
            for (auto &i: clients) {
                if (i->userName() == to) {
                    sendJson(i, TransferProtocol::sendMessage(msg, sender->userName(), i->userName()));
                    return;
                }
            }
        } else if (to == TransferProtocol::ALL) {
            for (auto &i: clients) {
                if (i != sender) {
                    sendJson(i, TransferProtocol::sendMessage(msg, sender->userName(), TransferProtocol::ALL));
                }
            }
            return;
        } else if (to == TransferProtocol::SERVER) {
            emit logMessage(tr("%1 :: %2").arg(sender->userName(), msg));
        }
    }
}

void ChatTcpServer::userDisconnected(ServerWorker *sender) {
    int ind = (int) clients.indexOf(sender);
    if (ind == -1) {
        return;
    }
    clients.remove(ind);
    for (auto &i: clients) {
        sendJson(i, TransferProtocol::sendDeadGuyInfo({sender->userName()}));
    }
    emit logMessage(tr("INFO :: %1 disconnected").arg(sender->userName()));
}

void ChatTcpServer::userError(ServerWorker *sender) {
    emit logMessage(tr("Error from ") + sender->userName());
}

void ChatTcpServer::stopServer() {
    for (ServerWorker *worker : clients) {
        worker->disconnectFromClient();
    }
    close();
    emit logMessage("INFO :: server stopped");
}
