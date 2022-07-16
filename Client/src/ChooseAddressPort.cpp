#include "ChooseAddressPort.h"
#include "ui_chooseAddressPort.h"
#include <QMessageBox>
#include <QKeyEvent>


ChooseAddressPort::ChooseAddressPort(QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseAddressPort) {
    ui->setupUi(this);
    setModal(true);
    setObjectName("MainWindow");

    connect(ui->pushButton_accept, &MyButton::clicked, this, &ChooseAddressPort::accept);
    connect(ui->pushButton_cancel, &MyButton::clicked, this, &ChooseAddressPort::reject);
}

ChooseAddressPort::~ChooseAddressPort() {
    delete ui;
}

QStringList ChooseAddressPort::getAddressPort(QWidget *parent, bool *ok) {
    auto *dialog = new ChooseAddressPort(parent);
    QStringList list;

    const int ret = dialog->exec();
    if (ok) {
        *ok = (bool) ret;
    }
    if (ret) {
        list << dialog->ui->lineEdit_address->text() << dialog->ui->lineEdit_port->text();
    }
    dialog->deleteLater();
    return list;
}

void ChooseAddressPort::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Enter) {
        if (!ui->pushButton_accept->hasFocus() && !ui->pushButton_cancel->hasFocus()) {
            ui->pushButton_accept->setFocus();
        }
    }
    QDialog::keyPressEvent(event);
}
