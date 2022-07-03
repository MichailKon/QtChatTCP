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

    socket = new QTcpSocket(this);

    connect(this, &mainwindow::newMessage, this, &mainwindow::showMessage);
    connect(socket, &QTcpSocket::readyRead, this, &mainwindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &mainwindow::discardSocket);
    connect(ui->pushButton, &QPushButton::clicked, this, &mainwindow::sendMessageButtonClicked);

    socket->connectToHost(QHostAddress::LocalHost, 8080);

    if (socket->waitForConnected()) {
        ui->label->setText(tr("Connected to server"));
    } else {
        QMessageBox::critical(this, "Client", QString("The following error occurred: %1.").arg(socket->errorString()));
        exit(EXIT_FAILURE);
    }
}

mainwindow::~mainwindow() {
    if (socket->isOpen()) {
        socket->close();
    }
    delete ui;
}

void mainwindow::showMessage(const QString &msg) {
    ui->textBrowser->append(msg);
}

void mainwindow::sendMessageButtonClicked() {
    if (!socket) return;
    if (!socket->isOpen()) return;
    QString msg = ui->lineEdit->text();

    QJsonObject message;
    message.insert("msg", msg);
    message.insert("type", TransferProtocol::NewMessage);
    message.insert("to", ui->comboBox->currentText());
    showMessage(QString("Message sent to %1: %2").arg(message["to"].toString(), msg));

    QDataStream stream(socket);
    stream << message;

    ui->lineEdit->clear();
}

void mainwindow::discardSocket() {
    socket->deleteLater();
    socket = nullptr;
    ui->label->setText("Disconnected");
}

void mainwindow::readSocket() {
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

    if (!stream.commitTransaction()) {
        QMessageBox::critical(this, "Client", "WTF");
        return;
    }

    TransferProtocol::DATA_TYPES type = TransferProtocol::DATA_TYPES(data["type"].toInt());
    if (type == TransferProtocol::NewMessage) {
        if (!data.contains("from") || !data.contains("msg")) {
            return;
        }
        QString message = QString("%1 :: %2").arg(doc["from"].toString(), doc["msg"].toString());
        emit newMessage(message);
    } else if (type == TransferProtocol::NewUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        for (auto && i : desc) {
            int de = i.toInt();
            otherSockets.insert(de);
            ui->comboBox->addItem(QString::number(de));
        }
    } else if (type == TransferProtocol::DelUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        for (auto && i : desc) {
            int de = i.toInt();
            otherSockets.remove(de);
            ui->comboBox->removeItem(ui->comboBox->findText(QString::number(de)));
        }
    }
}
