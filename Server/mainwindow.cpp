#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "../TransferProtocol.h"


mainwindow::mainwindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainwindow) {
    setWindowTitle("Server");
    ui->setupUi(this);
    ui->comboBox->addItem(allChat);

    server = new QTcpServer();
    if (server->listen(QHostAddress::Any, 8080)) {
        connect(this, &mainwindow::newMessage, this, &mainwindow::showMessage);
        connect(server, &QTcpServer::newConnection, this, &mainwindow::newConnection);
        ui->statusBar->showMessage("Waiting...");
    } else {
        QMessageBox::critical(this, "Server",
                              QString("Unable to start the server: %1.").arg(server->errorString()));
        exit(EXIT_FAILURE);
    }

    connect(ui->pushButton, &QPushButton::clicked, this, &mainwindow::sendMessageButtonClicked);
}

mainwindow::~mainwindow() {
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

void mainwindow::newConnection() {
    while (server->hasPendingConnections()) {
        appendToSockets(server->nextPendingConnection());
    }
}

void mainwindow::appendToSockets(QTcpSocket *socket) {
    connect(socket, &QTcpSocket::readyRead, this, &mainwindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &mainwindow::discardSocket);

    QJsonArray curGuys;
    auto it = con2desc.begin();
    while (it != con2desc.end()) {
        TransferProtocol::sendNewGuyInfo(it.key(), {(int) socket->socketDescriptor()});
        curGuys.append(it.value());
        ++it;
    }
//    TransferProtocol::sendSocketDescriptor(socket);
    TransferProtocol::sendNewGuyInfo(socket, curGuys);

    con2desc[socket] = (int) socket->socketDescriptor();
    desc2con[(int) socket->socketDescriptor()] = socket;
    ui->comboBox->addItem(QString::number(socket->socketDescriptor()));
    showMessage(QString("INFO :: %1 connected").arg(socket->socketDescriptor()));
}

void mainwindow::showMessage(const QString &msg) {
    ui->textBrowser->append(msg);
}

void mainwindow::sendMessageButtonClicked() {
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();
    auto it = con2desc.begin();
    while (it != con2desc.end()) {
        if (ui->comboBox->currentText() == allChat ||
            ui->comboBox->currentText().toInt() == it.value()) {
            TransferProtocol::sendMessage(it.key(), msg, TransferProtocol::SERVER);
        }
        ++it;
    }
    showMessage(QString("SERVER :: %1").arg(msg));
}

void mainwindow::readSocket() {
    auto *socket = (QTcpSocket *) sender();

    QByteArray buffer;

    QDataStream stream(socket);
    stream.setVersion(QDataStream::Qt_6_3);

    stream.startTransaction();
    stream >> buffer;

    QJsonDocument doc = QJsonDocument::fromJson(buffer);
    if (doc.isNull() || !doc.isObject()) {
        return;
    }
    QJsonObject data = doc.object();
    if (!data.contains("type")) {
        return;
    }

    int type = TransferProtocol::DATA_TYPES(data["type"].toInt());

    if (!stream.commitTransaction()) {
        QMessageBox::critical(this, "Server", "WTF");
        return;
    }

    if (type == TransferProtocol::NewMessage) {
        if (!data.contains("to") || !data.contains("msg")) {
            return;
        }
        QString to = data["to"].toString();
        bool *isNum = new bool();
        int toDesc = to.toInt(isNum);
        if (*isNum) {
            if (!desc2con.contains(toDesc)) {
                TransferProtocol::sendMessage(socket,
                                              tr("User %1 disconnected before getting your message").arg(
                                                      data["to"].toString()),
                                              TransferProtocol::SERVER);
            }
            int desc = (int) socket->socketDescriptor();
            TransferProtocol::sendMessage(desc2con.value(toDesc), doc["msg"].toString(), desc);
        } else {
            QString message = QString("%1 :: %2");
            message = message.arg(QString::number(socket->socketDescriptor()), doc["msg"].toString());
            emit newMessage(message);
        }
        delete (isNum);
    }
}

void mainwindow::discardSocket() {
    auto *socket = (QTcpSocket *) sender();
    auto it = con2desc.find(socket);
    if (it != con2desc.end()) {
        int dead = it.value();
        showMessage(QString("INFO :: A client %1 has just left the room").arg(dead));

        auto it1 = con2desc.begin();
        while (it1 != con2desc.end()) {
            TransferProtocol::sendDeadGuyInfo(it1.key(), {dead});
            ++it1;
        }

        ui->comboBox->removeItem(ui->comboBox->findText(QString::number(dead)));
        desc2con.remove(dead);
        con2desc.remove(it.key());
    }
    socket->deleteLater();
}
