#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <QWidget>
#include <QtNetwork/QTcpSocket>


QT_BEGIN_NAMESPACE
namespace Ui { class mainwindow; }
QT_END_NAMESPACE

class mainwindow : public QWidget {
Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);

    ~mainwindow() override;

signals:
    void newMessage(QString);

private slots:
    void readSocket();
    void discardSocket();

    void showMessage(const QString &msg);
    void sendMessageButtonClicked();

private:
    Ui::mainwindow *ui;

    QTcpSocket *socket;
    QSet<int> otherSockets;
};


#endif //CLIENT_MAINWINDOW_H
