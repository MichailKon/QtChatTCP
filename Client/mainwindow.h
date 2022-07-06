#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <QWidget>
#include <QtNetwork/QTcpSocket>

class QTextBrowser;

class QShortcut;


QT_BEGIN_NAMESPACE
namespace Ui { class mainwindow; }
QT_END_NAMESPACE

class mainwindow : public QWidget {
Q_OBJECT

    enum SHOW_MESSAGE_TYPES {
        FromServerMessage,
        SentMessage,
        NormalMessage
    };

public:
    explicit mainwindow(QWidget *parent = nullptr);

    ~mainwindow() override;

signals:

    void newMessage(QString msg, SHOW_MESSAGE_TYPES type, int ind);

    void newData(const QJsonDocument &doc);

private slots:

    void readSocket();

    void discardSocket();

    void showMessage(QString msg, mainwindow::SHOW_MESSAGE_TYPES from, int ind);

    void sendMessageButtonClicked();

    void handleData(const QJsonDocument &doc);

private:
    Ui::mainwindow *ui;

    QTcpSocket *socket;
    QVector<QVariant> otherSockets;
    QVector<QTextBrowser *> textBrowsers;
    QShortcut *keyEnter;
    qint32 nextBlock = 0;
    const QString allChat = "All";
    const QString serverMessage = "Server";

    void createTabWidget(const QString &title);
};


#endif //CLIENT_MAINWINDOW_H
