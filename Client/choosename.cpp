//
// Created by Михаил on 06.07.2022.
//

// You may need to build the project (run Qt uic code generator) to get "ui_chooseName.h" resolved

#include "choosename.h"
#include "ui_chooseName.h"
#include <QMessageBox>


chooseName::chooseName(QWidget *parent) :
        QDialog(parent), ui(new Ui::chooseName) {
    ui->setupUi(this);
    setModal(true);
    setWindowFlags(windowFlags() ^ Qt::WindowCloseButtonHint);

    connect(ui->pushButton, &QPushButton::clicked, this, &chooseName::close);
}

chooseName::~chooseName() {
    delete ui;
}

void chooseName::close() {
    if (ui->lineEdit->text().isEmpty()) {
        QMessageBox::critical(this, "ERROR", "You have to pass a name");
        return;
    }
    emit closed(ui->lineEdit->text());
    accept();
}