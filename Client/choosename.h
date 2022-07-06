//
// Created by Михаил on 06.07.2022.
//

#ifndef CLIENT_CHOOSENAME_H
#define CLIENT_CHOOSENAME_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class chooseName; }
QT_END_NAMESPACE

class chooseName : public QDialog {
Q_OBJECT

public:
    explicit chooseName(QWidget *parent = nullptr);

    ~chooseName() override;

signals:

    void closed(const QString &name);

private slots:

    void close();

private:
    Ui::chooseName *ui;
};


#endif //CLIENT_CHOOSENAME_H
