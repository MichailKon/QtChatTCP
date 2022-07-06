#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "choosename.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QShortcut>
#include <QTextBrowser>
#include "../TransferProtocol.h"


mainwindow::mainwindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainwindow) {
    auto diag = new chooseName(this);
    connect(diag, &chooseName::closed, this, &mainwindow::gotName);
    diag->show();

    ui->setupUi(this);

    otherSockets.push_back(TransferProtocol::ALL);
    createTabWidget(TransferProtocol::ALL);
    otherSockets.push_back(TransferProtocol::SERVER);
    createTabWidget(TransferProtocol::SERVER);

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

void mainwindow::showMessage(QString msg, SHOW_MESSAGE_TYPES from, int ind) {
    if (from == FromServerMessage) {
        msg = tr("<span style=\"color:RED\">%1</span>").arg(msg);
    } else if (from == SentMessage) {
        msg = tr("<span style=\"color:BLUE\">%1</span>").arg(msg);
    }
    textBrowsers[ind]->append(msg);
}

void mainwindow::sendMessageButtonClicked() {
    if (!socket) return;
    if (!socket->isOpen()) return;
    if (ui->lineEdit->text().isEmpty()) {
        return;
    }
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();

    int ind = ui->tabWidget->currentIndex();
    QString curTabText = ui->tabWidget->tabText(ind);
    if (curTabText == allChat) {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        TransferProtocol::sendMessage(socket, msg, (int) socket->socketDescriptor(), TransferProtocol::ALL);
    } else if (curTabText == serverMessage) {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        TransferProtocol::sendMessage(socket, msg, (int) socket->socketDescriptor(), TransferProtocol::SERVER);
    } else {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        TransferProtocol::sendMessage(socket, msg, (int) socket->socketDescriptor(), otherSockets[ind]);
    }
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
        QString from = doc["from"].toString();
        QString message = QString("%1 :: %2").arg(otherNames.value(from, from), doc["msg"].toString());
        int ind = (int) otherSockets.indexOf(from);
        if (ind == -1) {
            return;
        }
        if (data.value("to").toString() == TransferProtocol::ALL) {
            ind = (int) otherSockets.indexOf(TransferProtocol::ALL);
            if (ind == -1) {
                QMessageBox::critical(this, "ERROR", "Can't find chat with other people");
                return;
            }
        }
        emit newMessage(message,
                        doc["from"].toString() == TransferProtocol::SERVER ? FromServerMessage : NormalMessage,
                        ind);
    } else if (type == TransferProtocol::NewUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        QJsonArray names;
        if (data.contains("name")) {
            names = data["name"].toArray();
        }
        if (!names.empty() && desc.size() != names.size()) {
            close();
        }
        for (int i = 0; i < desc.size(); i++) {
            QString de = desc[i].toString();
            otherSockets.push_back(de);
            createTabWidget(de);
            if (!names.empty()) {
                setName(de, names[i].toString());
                otherNames[de] = names[i].toString();
            }
        }
    } else if (type == TransferProtocol::DelUsers) {
        if (!data.contains("descriptor")) {
            return;
        }
        QJsonArray desc = doc["descriptor"].toArray();
        for (auto &&i: desc) {
            QString de = i.toString();
            int ind = (int) otherSockets.indexOf(de);
            ui->tabWidget->removeTab(ind);
            otherSockets.remove(ind);
            delete textBrowsers[ind];
            textBrowsers.remove(ind);
            otherNames.remove(de);
        }
    } else if (type == TransferProtocol::YourSocketDescriptor) {
        if (!data.contains("descriptor")) {
            return;
        }
        setWindowTitle(tr("Client: %1").arg(doc["descriptor"].toInt()));
    } else if (type == TransferProtocol::NewUserName) {
        if (!data.contains("from") || !data.contains("name")) {
            return;
        }
        otherNames[data["from"].toString()] = data["name"].toString();
        setName(data["from"].toString(), data["name"].toString());
    }
}

void mainwindow::createTabWidget(const QString &title) {
    auto *tab = new QWidget();
    auto *grid = new QGridLayout();
    tab->setLayout(grid);
    auto *browser = new QTextBrowser();
    grid->addWidget(browser);
    ui->tabWidget->addTab(tab, title);
    textBrowsers.push_back(browser);
}

void mainwindow::gotName(const QString &name) {
    setWindowTitle(tr("Client: %1").arg(name));
    TransferProtocol::sendUserName(socket, "", name);
}

void mainwindow::setName(const QString &from, const QString &name) {
    int ind = (int) otherSockets.indexOf(from);
    if (ind == -1) {
        return;
    }
    ui->tabWidget->setTabText(ind, name);
}
