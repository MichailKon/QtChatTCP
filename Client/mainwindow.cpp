#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../TransferProtocol.h"


mainwindow::mainwindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainwindow) {
    ui->setupUi(this);
    ui->comboBox->addItem(TransferProtocol::SERVER);
    nextBlock = 0;

    socket = new QTcpSocket(this);

    socket->connectToHost(QHostAddress::LocalHost, 8080);
    if (socket->waitForConnected()) {
        ui->label->setText(tr("Connected to server"));
    } else {
        QMessageBox::critical(this, "Client", QString("The following error occurred: %1.").arg(socket->errorString()));
        exit(EXIT_FAILURE);
    }

    connect(this, &mainwindow::newData, this, &mainwindow::handleData);
    connect(this, &mainwindow::newMessage, this, &mainwindow::showMessage);
    connect(socket, &QTcpSocket::readyRead, this, &mainwindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &mainwindow::discardSocket);
    connect(ui->pushButton, &QPushButton::clicked, this, &mainwindow::sendMessageButtonClicked);
    keyEnter = new QShortcut(this);
    keyEnter->setKey(Qt::Key_Return);
    connect(keyEnter, &QShortcut::activated, ui->pushButton, &QPushButton::click);
}

mainwindow::~mainwindow() {
    if (socket->isOpen()) {
        socket->close();
    }
    delete ui;
}

void mainwindow::showMessage(QString msg, SHOW_MESSAGE_TYPES from) {
    if (from == FromServerMessage) {
        msg = tr("<span style=\"color:RED\">%1</span>").arg(msg);
    } else if (from == SentMessage) {
        msg = tr("<span style=\"color:BLUE\">%1</span>").arg(msg);
    }
    ui->textBrowser->append(msg);
}

void mainwindow::sendMessageButtonClicked() {
    if (!socket) return;
    if (!socket->isOpen()) return;
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();
    showMessage(QString("YOU -> %1: %2").arg(ui->comboBox->currentText(), msg), SentMessage);
    TransferProtocol::sendMessage(socket, msg, (int) socket->socketDescriptor(), ui->comboBox->currentText());
}

void mainwindow::discardSocket() {
    socket->deleteLater();
    socket = nullptr;
    ui->label->setText("Disconnected");
}

void mainwindow::readSocket() {
    QByteArray buffer;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_3);
    nextBlock = 0;
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
        emit newData(doc);

        buffer.clear();
        nextBlock = 0;
    }
}

void mainwindow::handleData(const QJsonDocument &doc) {
    if (doc.isNull() || !doc.isObject()) {
        return;
    }
    QJsonObject data = doc.object();
    if (!data.contains("type")) {
        return;
    }

    TransferProtocol::DATA_TYPES type = TransferProtocol::DATA_TYPES(data["type"].toInt());
    if (type == TransferProtocol::NewMessage) {
        if (!data.contains("from") || !data.contains("msg")) {
            return;
        }
        QString message = QString("%1 :: %2").arg(doc["from"].toString(), doc["msg"].toString());
        emit newMessage(message,
                        doc["from"].toString() == TransferProtocol::SERVER ? FromServerMessage : NormalMessage);
    } else if (type == TransferProtocol::NewUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        for (auto &&i: desc) {
            int de = i.toInt();
            otherSockets.insert(de);
            ui->comboBox->addItem(QString::number(de));
        }
    } else if (type == TransferProtocol::DelUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        for (auto &&i: desc) {
            int de = i.toInt();
            otherSockets.remove(de);
            ui->comboBox->removeItem(ui->comboBox->findText(QString::number(de)));
        }
    } else if (type == TransferProtocol::YourSocketDescriptor) {
        if (!data.contains("descriptor")) {
            return;
        }
        setWindowTitle(tr("Client: %1").arg(doc["descriptor"].toInt()));
    }
}
