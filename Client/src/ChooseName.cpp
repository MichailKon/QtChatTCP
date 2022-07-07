#include "ChooseName.h"
#include "ui_chooseName.h"
#include <QMessageBox>


ChooseName::ChooseName(QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseName) {
    ui->setupUi(this);
    setModal(true);
    setWindowFlags(windowFlags() ^ Qt::WindowCloseButtonHint);

    connect(ui->pushButton, &QPushButton::clicked, this, &ChooseName::close);
}

ChooseName::~ChooseName() {
    delete ui;
}

void ChooseName::close() {
    if (ui->lineEdit->text().isEmpty()) {
        QMessageBox::critical(this, "ERROR", "You have to pass a name");
        return;
    }
    emit closed(ui->lineEdit->text());
    accept();
}