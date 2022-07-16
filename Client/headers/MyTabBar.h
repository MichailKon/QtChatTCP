#ifndef CLIENT_MYTABBAR_H
#define CLIENT_MYTABBAR_H

#include <QTabBar>

class MyTabBar : public QTabBar {
    Q_OBJECT

public:
    MyTabBar(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};

#endif //CLIENT_MYTABBAR_H
