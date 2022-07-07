#include "VerticalTabWidget.h"
#include "VerticalTabBar.h"

VerticalTabWidget::VerticalTabWidget(QWidget *parent) : QTabWidget(parent) {
    setTabBar(new VerticalTabBar);
    setTabPosition(TabPosition::West);
}
