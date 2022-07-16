#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ChooseAddressPort.h"
#include "TransferProtocol.h"
#include "ClientConnection.h"
#include "ChooseName.h"
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
#include <QKeyEvent>


MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow), connection(new ClientConnection(this)) {
    ui->setupUi(this);
    setObjectName("MainWindow");

    QFile file("qss/tabWidget.qss");
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
    tab->setObjectName("messageBox");
    auto *grid = new QGridLayout();
    tab->setLayout(grid);
    auto *browser = new QTextBrowser();
    grid->addWidget(browser);
    ui->tabWidget->addTab(tab, title);
    textBrowsers.push_back(browser);
}

void MainWindow::attemptConnection() {
    if (connection->isLoggedIn()) {
        QMessageBox msgBox(this);
        msgBox.setObjectName("MainWindow");
        msgBox.setText("Отключиться?");
        msgBox.setWindowTitle(tr("Disconnect"));
        auto pButtonYes = new MyButton("Да");
        pButtonYes->set_bgColor("#777");
        pButtonYes->set_textColor("white");
        auto pButtonNo = new MyButton("Нет");
        pButtonNo->set_bgColor("#777");
        pButtonNo->set_textColor("white");
        msgBox.addButton(pButtonNo, QMessageBox::NoRole);
        msgBox.addButton(pButtonYes, QMessageBox::NoRole); // trash but nice order
        msgBox.exec();
        if (msgBox.clickedButton() == pButtonYes) {
            connection->disconnect();
        }
        pButtonYes->deleteLater();
        pButtonNo->deleteLater();
        ui->lineEdit->setFocus();
        return;
    }
    const QStringList connectionData = ChooseAddressPort::getAddressPort(this);
    if (connectionData.size() != 2) {
        return;
    }
    auto [address, port] = std::tie(connectionData[0], connectionData[1]);
    if (address.isEmpty() || port.isEmpty()) {
        return;
    }
    ui->pushButton_connect->setText("Отключиться");
    ui->pushButton_connect->setDisabled(true);
    ui->statusBar->showMessage("Попытка подключения");
    connection->connectToServer(QHostAddress(address), 8080);
    ui->pushButton_connect->setFocus();
}

void MainWindow::connectedToServer() {
    ui->pushButton_connect->setEnabled(true);
    const QString name = ChooseName::getName(this);

    if (name.isEmpty()) {
        showMessageBox("Error", "Необходимо ввести имя", QMessageBox::Critical);
        return connection->disconnect();
    }
    ui->statusBar->showMessage("Попытка логина");
    attemptLogin(name);
}

void MainWindow::attemptLogin(const QString &userName) {
    connection->login(userName);
    setWindowTitle("Client: " + userName);
}

void MainWindow::loggedIn() {
    ui->pushButton_sendMessage->show();
    ui->lineEdit->setEnabled(true);
    ui->tabWidget->setEnabled(true);
    ui->statusBar->showMessage("Подключено");
    ui->lineEdit->setFocus();
}

void MainWindow::loginFailed(const QString &reason) {
    showMessageBox("Error", reason, QMessageBox::Critical);
    connectedToServer();
}

void MainWindow::messageReceived(const QString &msg, const QString &from, const QString &to) {
    int ind = (int) otherSockets.indexOf(from);
    if (to == TransferProtocol::ALL) {
        ind = (int) otherSockets.indexOf(TransferProtocol::ALL);
        if (ind == -1) {
            showMessageBox("Error", "Нет чата со всеми", QMessageBox::Critical);
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
    if (msg.isEmpty()) {
        return;
    }
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
    setWindowTitle("Client");
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
            showMessageBox(tr("Disconnected"), tr("The host terminated the connection"), QMessageBox::Warning);
            return; // handled by disconnectedFromServer
        case QAbstractSocket::ConnectionRefusedError:
            showMessageBox(tr("Error"), tr("The host refused the connection"), QMessageBox::Critical);
            break;
        case QAbstractSocket::ProxyConnectionRefusedError:
            showMessageBox(tr("Error"), tr("The proxy refused the connection"), QMessageBox::Critical);
            break;
        case QAbstractSocket::ProxyNotFoundError:
            showMessageBox(tr("Error"), tr("Could not find the proxy"), QMessageBox::Critical);
            break;
        case QAbstractSocket::HostNotFoundError:
            showMessageBox(tr("Error"), tr("Could not find the server"), QMessageBox::Critical);
            break;
        case QAbstractSocket::SocketAccessError:
            showMessageBox(tr("Error"), tr("You don't have permissions to execute this operation"),
                           QMessageBox::Critical);
            break;
        case QAbstractSocket::SocketResourceError:
            showMessageBox(tr("Error"), tr("Too many connections opened"), QMessageBox::Critical);
            break;
        case QAbstractSocket::SocketTimeoutError:
            showMessageBox(tr("Error"), tr("Operation timed out"), QMessageBox::Warning);
            return;
        case QAbstractSocket::ProxyConnectionTimeoutError:
            showMessageBox(tr("Error"), tr("Proxy timed out"), QMessageBox::Critical);
            break;
        case QAbstractSocket::NetworkError:
            showMessageBox(tr("Error"), tr("Unable to reach the network"), QMessageBox::Critical);
            break;
        case QAbstractSocket::UnknownSocketError:
            showMessageBox(tr("Error"), tr("An unknown error occurred"), QMessageBox::Critical);
            break;
        case QAbstractSocket::UnsupportedSocketOperationError:
            showMessageBox(tr("Error"), tr("Operation not supported"), QMessageBox::Critical);
            break;
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            showMessageBox(tr("Error"), tr("Your proxy requires authentication"), QMessageBox::Critical);
            break;
        case QAbstractSocket::ProxyProtocolError:
            showMessageBox(tr("Error"), tr("Proxy communication failed"), QMessageBox::Critical);
            break;
        case QAbstractSocket::TemporaryError:
        case QAbstractSocket::OperationError:
            showMessageBox(tr("Error"), tr("Operation failed, please try again"), QMessageBox::Warning);
            return;
        default:
            Q_UNREACHABLE();
    }
    ui->pushButton_connect->setText("Подключиться");
    ui->pushButton_sendMessage->hide();
    ui->lineEdit->setDisabled(true);
    ui->tabWidget->setDisabled(true);
    ui->pushButton_connect->setEnabled(true);
    ui->statusBar->showMessage("Ожидание подключения");
}

void MainWindow::prepareTabWidget() {
    otherSockets.clear();
    for (auto &i: textBrowsers) {
        i->deleteLater();
    }
    textBrowsers.clear();
    ui->tabWidget->clear();

    otherSockets.push_back(TransferProtocol::ALL);
    createTabWidget(TransferProtocol::ALL);
    otherSockets.push_back(TransferProtocol::SERVER);
    createTabWidget(TransferProtocol::SERVER);
}

void MainWindow::showMessageBox(const QString &title, const QString &text, const QMessageBox::Icon &icon) {
    QMessageBox box(this);
    box.setObjectName("MainWindow");
    box.setIcon(icon);
    box.setText(text);
    auto *btn = new MyButton("OK");
    box.addButton(btn, QMessageBox::YesRole);
    box.exec();
    btn->deleteLater();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (ui->pushButton_connect->hasFocus() && event->key() == Qt::Key_Return) {
        ui->pushButton_connect->click();
        return;
    }
    QWidget::keyPressEvent(event);
}
