//
// Created by Михаил on 06.07.2022.
//

#ifndef CLIENT_CHOOSEADDRESSPORT_H
#define CLIENT_CHOOSEADDRESSPORT_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ChooseAddressPort; }
QT_END_NAMESPACE

class ChooseName : public QDialog {
Q_OBJECT

public:
    explicit ChooseName(QWidget *parent = nullptr);

    ~ChooseName() override;

    static QStringList getAddressPort(QWidget *parent, bool *ok = nullptr);

private:
    Ui::ChooseAddressPort *ui;
};


#endif //CLIENT_CHOOSEADDRESSPORT_H
