#include "ChooseName.h"
#include "ui_chooseName.h"


ChooseName::ChooseName(QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseName) {
    ui->setupUi(this);
    setModal(true);
    setObjectName("MainWindow");

    connect(ui->pushButton_ok, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->lineEdit, &QLineEdit::returnPressed, ui->pushButton_ok, &QPushButton::click);
    ui->lineEdit->setFocus();
}

ChooseName::~ChooseName() {
    delete ui;
}

QString ChooseName::getName(QWidget *parent, bool *ok) {
    auto *dialog = new ChooseName(parent);
    QString res;

    const int ret = dialog->exec();
    if (ok) {
        *ok = (bool) ret;
    }
    if (ret) {
        res = dialog->ui->lineEdit->text();
    }
    dialog->deleteLater();
    return res;
}