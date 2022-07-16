#ifndef CLIENT_CHOOSENAME_H
#define CLIENT_CHOOSENAME_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ChooseName; }
QT_END_NAMESPACE

class ChooseName : public QDialog {
Q_OBJECT

public:
    explicit ChooseName(QWidget *parent = nullptr);

    ~ChooseName() override;

    static QString getName(QWidget *parent, bool *ok = nullptr);

private:
    Ui::ChooseName *ui;
};


#endif //CLIENT_CHOOSENAME_H
