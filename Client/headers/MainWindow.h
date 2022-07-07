#ifndef CLIENT_MAINWINDOW_H
#define CLIENT_MAINWINDOW_H

#include <QWidget>

class QTextBrowser;

class QShortcut;

class QTcpSocket;

class VerticalTabWidget;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget {
Q_OBJECT

    enum SHOW_MESSAGE_TYPES {
        FromServerMessage,
        SentMessage,
        NormalMessage
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

signals:

    void newMessage(QString msg, SHOW_MESSAGE_TYPES type, int ind);

    void newData(const QJsonDocument &doc);

private slots:

    void readSocket();

    void discardSocket();

    void showMessage(QString msg, MainWindow::SHOW_MESSAGE_TYPES from, int ind);

    void sendMessageButtonClicked();

    void handleData(const QJsonDocument &doc);

    void gotName(const QString &name);

private:
    Ui::MainWindow *ui;

    QTcpSocket *socket;
    QVector<QString> otherSockets;
    QVector<QTextBrowser *> textBrowsers;
    QShortcut *keyEnter;
    QShortcut *keyCtrlTab;
    QMap<QString, QString> otherNames;
    qint32 nextBlock = 0;
    const QString allChat = "All";
    const QString serverMessage = "Server";

    void createTabWidget(const QString &title);

    void setName(const QString &from, const QString &name);
};


#endif //CLIENT_MAINWINDOW_H
