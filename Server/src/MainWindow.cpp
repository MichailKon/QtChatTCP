#include <QMessageBox>
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "ChatTcpServer.h"


MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::MainWindow), server(new ChatTcpServer) {
    ui->setupUi(this);

    ui->pushButton->setText(turnOnText);
    ui->statusBar->showMessage("Server stopped");

    connect(server, &ChatTcpServer::logMessage, this, &MainWindow::logMessage);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::buttonClicked);
}

MainWindow::~MainWindow() {
    delete ui;
    delete server;
}

void MainWindow::turnServerOn() {
    if (server->isListening()) {
        return;
    }
    if (server->listen(QHostAddress(ui->lineEdit_address->text()), ui->lineEdit_port->text().toInt())) {
        ui->pushButton->setText(turnOffText);
        ui->lineEdit_address->setDisabled(true);
        ui->lineEdit_port->setDisabled(true);
        ui->statusBar->showMessage("Server is working");
        logMessage("INFO :: Server is launched");
    } else {
        QMessageBox::critical(this, "ERROR", "Can't launch the server: " + server->errorString());
    }
}

void MainWindow::turnServerOff() {
    if (!server->isListening()) {
        return;
    }
    server->stopServer();
    ui->pushButton->setText(turnOnText);
    ui->lineEdit_address->setEnabled(true);
    ui->lineEdit_port->setEnabled(true);
    ui->statusBar->showMessage("Server stopped");
}

void MainWindow::buttonClicked() {
    if (ui->pushButton->text() == turnOnText) {
        turnServerOn();
    } else if (ui->pushButton->text() == turnOffText) {
        turnServerOff();
    }
}

void MainWindow::logMessage(const QString &message) {
    ui->textBrowser->append(message);
}