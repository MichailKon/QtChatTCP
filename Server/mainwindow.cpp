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
    nextBlock = 0;

    server = new QTcpServer();
    if (server->listen(QHostAddress::Any, 8080)) {
        connect(this, &mainwindow::newMessage, this, &mainwindow::showMessage);
        connect(this, &mainwindow::newData, this, &mainwindow::handleData);
        connect(server, &QTcpServer::newConnection, this, &mainwindow::newConnection);
        ui->statusBar->showMessage("Waiting...");
    } else {
        QMessageBox::critical(this, "Server",
                              QString("Unable to start the server: %1.").arg(server->errorString()));
        exit(EXIT_FAILURE);
    }

    keyEnter = new QShortcut(this);
    keyEnter->setKey(Qt::Key_Return);
    connect(keyEnter, &QShortcut::activated, ui->pushButton, &QPushButton::click);
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
    keyEnter->deleteLater();
}

void mainwindow::newConnection() {
    while (server->hasPendingConnections()) {
        appendToSockets(server->nextPendingConnection());
    }
}

void mainwindow::appendToSockets(QTcpSocket *socket) {
    connect(socket, &QTcpSocket::readyRead, this, &mainwindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &mainwindow::discardSocket);

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
    ui->comboBox->addItem(QString::number(socket->socketDescriptor()));
    showMessage(QString("INFO :: %1 connected").arg(socket->socketDescriptor()), InfoMessage);
}

void mainwindow::showMessage(QString msg, SHOW_MESSAGE_TYPES from) {
    if (from == InfoMessage) {
        msg = tr("<span style=\"color:RED\">%1</span>").arg(msg);
    } else if (from == SentMessage) {
        msg = tr("<span style=\"color:BLUE\">%1</span>").arg(msg);
    }
    ui->textBrowser->append(msg);
}

void mainwindow::sendMessageButtonClicked() {
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();
    auto it = con2desc.begin();
    while (it != con2desc.end()) {
        if (ui->comboBox->currentText() == allChat ||
            ui->comboBox->currentText().toInt() == it.value()) {
            TransferProtocol::sendMessage(it.key(), msg, TransferProtocol::SERVER, it.value());
        }
        ++it;
    }
    if (ui->comboBox->currentText() == allChat) {
        showMessage(QString("Server -> ALL: %1").arg(msg), SentMessage);
    } else {
        showMessage(QString("Server -> %1: %2").arg(ui->comboBox->currentText(), msg), SentMessage);
    }
}

void mainwindow::readSocket() {
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

void mainwindow::discardSocket() {
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

        ui->comboBox->removeItem(ui->comboBox->findText(dead));
        desc2con.remove(dead.toInt());
        con2desc.remove(it.key());
    }
    socket->deleteLater();
}

void mainwindow::handleData(QTcpSocket *socket, const QJsonDocument &doc) {
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
    }
}
