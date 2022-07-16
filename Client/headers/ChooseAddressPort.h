#ifndef CLIENT_CHOOSEADDRESSPORT_H
#define CLIENT_CHOOSEADDRESSPORT_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ChooseAddressPort; }
QT_END_NAMESPACE

class ChooseAddressPort : public QDialog {
Q_OBJECT

public:
    explicit ChooseAddressPort(QWidget *parent = nullptr);

    ~ChooseAddressPort() override;

    static QStringList getAddressPort(QWidget *parent, bool *ok = nullptr);

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    Ui::ChooseAddressPort *ui;
};


#endif //CLIENT_CHOOSEADDRESSPORT_H
