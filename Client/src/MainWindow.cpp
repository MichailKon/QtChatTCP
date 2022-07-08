#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ChooseAddressPort.h"
#include "VerticalTabBar.h"
#include "TransferProtocol.h"
#include "VerticalTabWidget.h"
#include "ClientConnection.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QShortcut>
#include <QTextBrowser>
#include <QConstOverload>
#include <QtNetwork/QAbstractSocket>
#include <QFile>
#include <QtNetwork/QTcpSocket>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow), connection(new ClientConnection(this)) {
    ui->setupUi(this);

    QFile file("css/tabWidget.css");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);
    prepareTabWidget();

    keyCtrlTab = new QShortcut(this);
    keyCtrlTab->setKey(Qt::Key_Control | Qt::Key_T);
    connect(keyCtrlTab, &QShortcut::activated, ui->tabWidget, &QTabWidget::nextInFocusChain);

    connect(connection, &ClientConnection::connected, this, &MainWindow::connectedToServer);
    connect(connection, &ClientConnection::loggedIn, this, &MainWindow::loggedIn);
    connect(connection, &ClientConnection::loginError, this, &MainWindow::loginFailed);
    connect(connection, &ClientConnection::newMessage, this, &MainWindow::messageReceived);
    connect(connection, &ClientConnection::disconnected, this, &MainWindow::disconnectedFromServer);
    connect(connection, &ClientConnection::error, this, &MainWindow::error);
    connect(connection, &ClientConnection::newUsers, this, &MainWindow::newUsers);
    connect(connection, &ClientConnection::delUsers, this, &MainWindow::delUsers);

    connect(ui->pushButton_connect, &QPushButton::clicked, this, &MainWindow::attemptConnection);

    connect(ui->pushButton_sendMessage, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    ui->pushButton_sendMessage->hide();

    connect(this, &MainWindow::newMessage, this, &MainWindow::showMessage);

    QSizePolicy sp_retain = ui->pushButton_sendMessage->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->pushButton_sendMessage->setSizePolicy(sp_retain);
    sp_retain = ui->pushButton_connect->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->pushButton_connect->setSizePolicy(sp_retain);

    ui->pushButton_sendMessage->hide();
    ui->lineEdit->setDisabled(true);
    ui->tabWidget->setDisabled(true);
    ui->statusBar->showMessage("Ожидание подключения");
}

MainWindow::~MainWindow() {
    delete ui;
    connection->deleteLater();
    keyCtrlTab->deleteLater();
    for (auto &i: textBrowsers) {
        i->deleteLater();
    }
}

void MainWindow::createTabWidget(const QString &title) {
    auto *tab = new QWidget();
    auto *grid = new QGridLayout();
    tab->setLayout(grid);
    auto *browser = new QTextBrowser();
    grid->addWidget(browser);
    ui->tabWidget->addTab(tab, title);
    textBrowsers.push_back(browser);
}

void MainWindow::attemptConnection() {
    if (connection->isLoggedIn()) {
        int res = QMessageBox::question(this, tr("Disconnect"), "Отключиться?",
                                        QMessageBox::Yes, QMessageBox::No);
        if (res == QMessageBox::Yes) {
            connection->disconnect();
        }
        return;
    }
    const QStringList connectionData = ChooseName::getAddressPort(this);
    if (connectionData.size() != 2) {
        return;
    }
    auto [address, port] = std::tie(connectionData[0], connectionData[1]);
    if (address.isEmpty() || port.isEmpty()) {
        return;
    }
    ui->pushButton_connect->setText("Отключиться");
    ui->statusBar->showMessage("Попытка подключения");
    connection->connectToServer(QHostAddress(address), 8080);
}

void MainWindow::connectedToServer() {
    const QString name = QInputDialog::getText(this, tr("Choose name"), tr("Логин"));
    if (name.isEmpty()) {
        QMessageBox::critical(this, "ERROR", "Необходимо ввести имя");
        return connection->disconnect();
    }
    ui->statusBar->showMessage("Попытка логина");
    attemptLogin(name);
}

void MainWindow::attemptLogin(const QString &userName) {
    connection->login(userName);
}

void MainWindow::loggedIn() {
    ui->pushButton_sendMessage->show();
    ui->lineEdit->setEnabled(true);
    ui->tabWidget->setEnabled(true);
    ui->statusBar->showMessage("Подключено");
}

void MainWindow::loginFailed(const QString &reason) {
    QMessageBox::critical(this, tr("Error"), reason);
    connectedToServer();
}

void MainWindow::messageReceived(const QString &msg, const QString &from, const QString &to) {
    int ind = (int) otherSockets.indexOf(from);
    if (to == TransferProtocol::ALL) {
        ind = (int) otherSockets.indexOf(TransferProtocol::ALL);
        if (ind == -1) {
            QMessageBox::critical(this, "ERROR", "Can't find chat with other people");
            return;
        }
    }
    if (ind == -1) {
        return;
    }
    emit newMessage(tr("%1 :: %2").arg(from, msg),
                    from == TransferProtocol::SERVER ? FromServerMessage : NormalMessage,
                    ind);
}

void MainWindow::showMessage(const QString &msg, SHOW_MESSAGE_TYPE messageType, int ind) {
    if (ind < 0 || ind >= textBrowsers.size()) {
        return;
    }
    QString message = msg;
    if (messageType == FromServerMessage) {
        message = tr("<span style=\"color:RED\">%1</span>").arg(msg);
    } else if (messageType == SentMessage) {
        message = tr("<span style=\"color:BLUE\">%1</span>").arg(msg);
    }
    textBrowsers[ind]->append(message);
}

void MainWindow::sendMessage() {
    QString msg = ui->lineEdit->text();
    ui->lineEdit->clear();
    int ind = ui->tabWidget->currentIndex();
    QString curTabText = ui->tabWidget->tabText(ind);
    if (curTabText == allChat) {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        connection->sendMessage(msg, TransferProtocol::ALL);
    } else if (curTabText == serverMessage) {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        connection->sendMessage(msg, TransferProtocol::SERVER);
    } else {
        showMessage(tr("YOU: %1").arg(msg), SentMessage, ind);
        connection->sendMessage(msg, curTabText);
    }
}

void MainWindow::disconnectedFromServer() {
    ui->pushButton_sendMessage->hide();
    ui->lineEdit->setDisabled(true);
    ui->tabWidget->setDisabled(true);
    ui->pushButton_connect->setText("Подключиться");
    ui->statusBar->showMessage("Ожидание подключения");
    prepareTabWidget();
}

void MainWindow::newUsers(const QJsonArray &names) {
    QStringList convNames;
    for (auto &&name: names) {
        convNames << name.toString();
        createTabWidget(name.toString());
        otherSockets.push_back(name.toString());
    }
    showMessage(tr("INFO :: %1 connected").arg(convNames.join(" ")), FromServerMessage,
                (int) otherSockets.indexOf(TransferProtocol::SERVER));
}

void MainWindow::delUsers(const QJsonArray &names) {
    QStringList convNames;
    for (auto &&name: names) {
        QString n = name.toString();
        int ind = (int) otherSockets.indexOf(n);
        if (ind == -1) {
            continue;
        }
        ui->tabWidget->removeTab(ind);
        delete textBrowsers[ind];
        textBrowsers.remove(ind);
        otherSockets.remove(ind);
        convNames << n;
    }
    showMessage(tr("INFO :: %1 disconnected").arg(convNames.join(" ")), FromServerMessage,
                (int) otherSockets.indexOf(TransferProtocol::SERVER));
}

void MainWindow::error(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        case QAbstractSocket::ProxyConnectionClosedError:
            QMessageBox::warning(this, tr("Disconnected"), tr("The host terminated the connection"));
            return; // handled by disconnectedFromServer
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::critical(this, tr("Error"), tr("The host refused the connection"));
            break;
        case QAbstractSocket::ProxyConnectionRefusedError:
            QMessageBox::critical(this, tr("Error"), tr("The proxy refused the connection"));
            break;
        case QAbstractSocket::ProxyNotFoundError:
            QMessageBox::critical(this, tr("Error"), tr("Could not find the proxy"));
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::critical(this, tr("Error"), tr("Could not find the server"));
            break;
        case QAbstractSocket::SocketAccessError:
            QMessageBox::critical(this, tr("Error"), tr("You don't have permissions to execute this operation"));
            break;
        case QAbstractSocket::SocketResourceError:
            QMessageBox::critical(this, tr("Error"), tr("Too many connections opened"));
            break;
        case QAbstractSocket::SocketTimeoutError:
            QMessageBox::warning(this, tr("Error"), tr("Operation timed out"));
            return;
        case QAbstractSocket::ProxyConnectionTimeoutError:
            QMessageBox::critical(this, tr("Error"), tr("Proxy timed out"));
            break;
        case QAbstractSocket::NetworkError:
            QMessageBox::critical(this, tr("Error"), tr("Unable to reach the network"));
            break;
        case QAbstractSocket::UnknownSocketError:
            QMessageBox::critical(this, tr("Error"), tr("An unknown error occurred"));
            break;
        case QAbstractSocket::UnsupportedSocketOperationError:
            QMessageBox::critical(this, tr("Error"), tr("Operation not supported"));
            break;
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            QMessageBox::critical(this, tr("Error"), tr("Your proxy requires authentication"));
            break;
        case QAbstractSocket::ProxyProtocolError:
            QMessageBox::critical(this, tr("Error"), tr("Proxy communication failed"));
            break;
        case QAbstractSocket::TemporaryError:
        case QAbstractSocket::OperationError:
            QMessageBox::warning(this, tr("Error"), tr("Operation failed, please try again"));
            return;
        default:
            Q_UNREACHABLE();
    }
    ui->pushButton_connect->setText("Подключиться");
    ui->pushButton_sendMessage->hide();
    ui->lineEdit->setDisabled(true);
    ui->tabWidget->setDisabled(true);
    ui->statusBar->showMessage("Ожидание подключения");
}

void MainWindow::prepareTabWidget() {
    otherSockets.clear();
    for (auto &i : textBrowsers) {
        delete i;
    }
    textBrowsers.clear();
    ui->tabWidget->clear();
//    while (ui->tabWidget->count()) {
//        ui->tabWidget->removeTab(0);
//    }

    otherSockets.push_back(TransferProtocol::ALL);
    createTabWidget(TransferProtocol::ALL);
    otherSockets.push_back(TransferProtocol::SERVER);
    createTabWidget(TransferProtocol::SERVER);
}
