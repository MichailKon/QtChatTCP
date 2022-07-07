#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "TransferProtocol.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>


MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow) {
    setWindowTitle("Server");
    ui->setupUi(this);
    nextBlock = 0;

    server = new QTcpServer();
    if (server->listen(QHostAddress::Any, 8080)) {
        connect(this, &MainWindow::newMessage, this, &MainWindow::showMessage);
        connect(this, &MainWindow::newData, this, &MainWindow::handleData);
        connect(server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
        ui->statusBar->showMessage("Waiting...");
    } else {
        QMessageBox::critical(this, "Server",
                              QString("Unable to start the server: %1.").arg(server->errorString()));
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow() {
    delete ui;

    auto it = desc2con.begin();
    while (it != desc2con.end()) {
        it.value()->close();
        it.value()->deleteLater();
        ++it;
    }

    desc2con.clear(), con2desc.clear();
    server->close();
    server->deleteLater();
}

void MainWindow::newConnection() {
    while (server->hasPendingConnections()) {
        appendToSockets(server->nextPendingConnection());
    }
}

void MainWindow::appendToSockets(QTcpSocket *socket) {
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);

    QJsonArray curGuys, curNames;
    auto it = con2desc.begin();
    while (it != con2desc.end()) {
        TransferProtocol::sendNewGuyInfo(it.key(), {QString::number(socket->socketDescriptor())});
        curGuys.append(QString::number(it.value()));
        curNames.append(con2name[it.key()]);
        ++it;
    }
    TransferProtocol::sendNewGuyInfo(socket, curGuys, curNames);
    TransferProtocol::sendSocketDescriptor(socket);

    con2desc[socket] = (int) socket->socketDescriptor();
    desc2con[(int) socket->socketDescriptor()] = socket;
    con2name[socket] = QString::number(socket->socketDescriptor());
    showMessage(QString("INFO :: %1 connected").arg(socket->socketDescriptor()), InfoMessage);
}

void MainWindow::showMessage(QString msg, SHOW_MESSAGE_TYPES from) {
    if (from == InfoMessage) {
        msg = tr("<span style=\"color:RED\">%1</span>").arg(msg);
    } else if (from == SentMessage) {
        msg = tr("<span style=\"color:BLUE\">%1</span>").arg(msg);
    }
    ui->textBrowser->append(msg);
}

void MainWindow::readSocket() {
    auto *socket = (QTcpSocket *) sender();

    QByteArray buffer;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_3);

    forever {
        if (nextBlock == 0) {
            if (socket->bytesAvailable() < sizeof(qint32)) {
                break;
            }
            in >> nextBlock;
        }
        if (socket->bytesAvailable() < nextBlock) {
            break;
        }
        in >> buffer;

        QJsonDocument doc = QJsonDocument::fromJson(buffer);
        emit newData(socket, doc);

        buffer.clear();
        nextBlock = 0;
    }
}

void MainWindow::discardSocket() {
    auto *socket = (QTcpSocket *) sender();
    auto it = con2desc.find(socket);
    if (it != con2desc.end()) {
        QString dead = QString::number(it.value());
        showMessage(QString("INFO :: %1 disconnected").arg(dead), InfoMessage);

        auto it1 = con2desc.begin();
        while (it1 != con2desc.end()) {
            TransferProtocol::sendDeadGuyInfo(it1.key(), {dead});
            ++it1;
        }

        desc2con.remove(dead.toInt());
        con2desc.remove(it.key());
    }
    socket->deleteLater();
}

void MainWindow::handleData(QTcpSocket *socket, const QJsonDocument &doc) {
    if (doc.isNull() || !doc.isObject()) {
        return;
    }
    QJsonObject data = doc.object();
    if (!data.contains("type")) {
        return;
    }

    int type = TransferProtocol::DATA_TYPES(data["type"].toInt());

    if (type == TransferProtocol::NewMessage) {
        if (!data.contains("to") || !data.contains("msg")) {
            return;
        }
        QString msg = doc["msg"].toString();
        QString to = data["to"].toString();
        if (to != TransferProtocol::SERVER && to != TransferProtocol::ALL) {
            bool *isNum = new bool();
            int toDesc = to.toInt(isNum);
            if (*isNum) {
                if (!desc2con.contains(toDesc)) {
                    TransferProtocol::sendMessage(socket,
                                                  tr("User %1 is not connect").arg(data["to"].toString()),
                                                  TransferProtocol::SERVER,
                                                  (int) socket->socketDescriptor());
                }
                int desc = (int) socket->socketDescriptor();
                TransferProtocol::sendMessage(desc2con.value(toDesc), msg, desc, toDesc);
            }
            delete (isNum);
        } else if (to == TransferProtocol::SERVER) {
            QString message = QString("%1 :: %2");
            message = message.arg(QString::number(socket->socketDescriptor()), msg);
            emit newMessage(message, NormalMessage);
        } else if (to == TransferProtocol::ALL) {
            auto it = con2desc.begin();
            while (it != con2desc.end()) {
                if (it.key() != socket) {
                    TransferProtocol::sendMessage(it.key(),
                                                  msg,
                                                  socket->socketDescriptor(), TransferProtocol::ALL);
                }
                ++it;
            }
        }
    } else if (type == TransferProtocol::NewUserName) {
        if (!data.contains("name")) {
            return;
        }
        QString name = data["name"].toString();
        con2name[socket] = name;
        auto it = con2desc.begin();
        while (it != con2desc.end()) {
            if (it.value() != socket->socketDescriptor()) {
                TransferProtocol::sendUserName(it.key(), socket->socketDescriptor(), name);
            }
            ++it;
        }
        showMessage(tr("%1 changed name to %2").arg(QString::number(socket->socketDescriptor()), name), InfoMessage);
    }
}
