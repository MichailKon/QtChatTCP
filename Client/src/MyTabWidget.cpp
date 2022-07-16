#include "MyTabWidget.h"
#include "MyTabBar.h"

MyTabWidget::MyTabWidget(QWidget *parent) : QTabWidget(parent) {
    setTabBar(new MyTabBar(this));
}
