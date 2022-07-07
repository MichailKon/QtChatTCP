#ifndef CLIENT_VERTICALTABBAR_H
#define CLIENT_VERTICALTABBAR_H

#include <QTabBar>

class VerticalTabBar : public QTabBar {
public:
    QSize tabSizeHint(int index) const override;
protected:
    void paintEvent(QPaintEvent *) override;
};

#endif //CLIENT_VERTICALTABBAR_H
