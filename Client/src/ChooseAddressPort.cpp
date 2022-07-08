#include "../headers/ChooseAddressPort.h"
#include "ui_chooseAddressPort.h"
#include <QMessageBox>


ChooseName::ChooseName(QWidget *parent) :
        QDialog(parent), ui(new Ui::ChooseAddressPort) {
    ui->setupUi(this);
    setModal(true);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChooseName::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChooseName::reject);
}

ChooseName::~ChooseName() {
    delete ui;
}

QStringList ChooseName::getAddressPort(QWidget *parent, bool *ok) {
    auto *dialog = new ChooseName(parent);
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
