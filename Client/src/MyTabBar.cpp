#include "MyTabBar.h"
#include <QStylePainter>
#include <QStyleOptionTab>

MyTabBar::MyTabBar(QWidget *parent) : QTabBar(parent) {}

void MyTabBar::paintEvent(QPaintEvent *event) {
    QStylePainter stylePaint(this);
    QStyleOptionTab opt;
    QPainter painter(this);

    for (int i = 0; i < count(); i++) {
        if (i != currentIndex()) {
            initStyleOption(&opt, i);
            stylePaint.save();
            stylePaint.drawControl(QStyle::CE_TabBarTabShape, opt);
            stylePaint.drawControl(QStyle::CE_TabBarTabLabel, opt);
            stylePaint.restore();
        }
    }

    initStyleOption(&opt, currentIndex());
    stylePaint.save();
    stylePaint.drawControl(QStyle::CE_TabBarTabShape, opt);
    stylePaint.drawControl(QStyle::CE_TabBarTabLabel, opt);
    stylePaint.restore();

    painter.end();
}
