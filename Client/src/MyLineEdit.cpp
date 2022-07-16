#include "MyLineEdit.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QPaintEvent>
#include <QSequentialAnimationGroup>

MyLineEdit::MyLineEdit(QWidget *parent) : MyLineEdit("", parent) {}

MyLineEdit::MyLineEdit(const QString &text, QWidget *parent) : QLineEdit(text, parent), m_bottomLinePercent(0),
                                                               m_topLinePercent(0), m_spanAngle(0) {
    connect(this, &MyLineEdit::objectChanged, this, &MyLineEdit::updater);

}

void MyLineEdit::updater() {
    update();
}

void MyLineEdit::paintEvent(QPaintEvent *event) {
    QLineEdit::paintEvent(event);

    QPainter painter(this);

    QPen pen("black");
    pen.setWidth(2);
    painter.setPen(pen);

    int width = QWidget::width(), height = QWidget::height();
    if (topLinePercent() > 0) {
        qreal w = needWidth();
        painter.drawLine(QLineF(width / 2. - w * topLinePercent(), 0.5, width / 2. + w * topLinePercent(), 0.5));
    }
    if (spanAngle() > 0) {
        painter.drawArc(QRectF(1, 0.5, 20, height - 0.5), 90 * 16, spanAngle());
        painter.drawArc(QRectF(width - 20 - 1.5, 0.5, 20, height - 0.5), 90 * 16, -spanAngle());
    }
    if (bottomLinePercent() > 0) {
        qreal w = needWidth();
        painter.drawLine(QLineF(10, height - 0.5, 10 + w * bottomLinePercent(), height - 0.5));
        painter.drawLine(QLineF(width - 10 - w * bottomLinePercent(), height - 0.5, width - 10, height - 0.5));
    }

    painter.end();
}

void MyLineEdit::focusInEvent(QFocusEvent *event) {
    QLineEdit::focusInEvent(event);

    auto *group = new QSequentialAnimationGroup(this);

    auto *topLineAnimation = new QPropertyAnimation(this, "m_topLinePercent", this);
    topLineAnimation->setStartValue(topLinePercent());
    topLineAnimation->setEndValue(1.);
    qreal part = topLineAnimation->startValue().toReal() / topLineAnimation->endValue().toReal();
    topLineAnimation->setDuration(std::max(0, (int)(200 * (1 - part))));
    group->addAnimation(topLineAnimation);

    auto *spanAngleAnimation = new QPropertyAnimation(this, "m_spanAngle", this);
    spanAngleAnimation->setStartValue(spanAngle());
    spanAngleAnimation->setEndValue(180 * 16);
    part = spanAngleAnimation->startValue().toReal() / spanAngleAnimation->endValue().toReal();
    spanAngleAnimation->setDuration(std::max(0, (int)(200 * (1 - part))));
    group->addAnimation(spanAngleAnimation);

    auto *bottomLineAnimation = new QPropertyAnimation(this, "m_bottomLinePercent", this);
    bottomLineAnimation->setStartValue(bottomLinePercent());
    bottomLineAnimation->setEndValue(1.);
    part = bottomLineAnimation->startValue().toReal() / bottomLineAnimation->endValue().toReal();
    bottomLineAnimation->setDuration(std::max(0, (int)(200 * (1 - part))));
    group->addAnimation(bottomLineAnimation);

    connect(group, &QSequentialAnimationGroup::finished, group, &QSequentialAnimationGroup::deleteLater);
    connect(this, &MyLineEdit::unfocused, group, &QSequentialAnimationGroup::deleteLater);
    group->start();
    emit focused();
}

void MyLineEdit::focusOutEvent(QFocusEvent *event) {
//    setSizeIncrement(-2, -2);

    auto *group = new QSequentialAnimationGroup(this);

    auto *bottomLineAnimation = new QPropertyAnimation(this, "m_bottomLinePercent", this);
    bottomLineAnimation->setStartValue(bottomLinePercent());
    bottomLineAnimation->setEndValue(0);
    qreal part = bottomLineAnimation->endValue().toReal() / bottomLineAnimation->startValue().toReal();
    bottomLineAnimation->setDuration(std::max(0, (int)(100 * (1 - part))));
    group->addAnimation(bottomLineAnimation);

    auto *spanAngleAnimation = new QPropertyAnimation(this, "m_spanAngle", this);
    spanAngleAnimation->setStartValue(spanAngle());
    spanAngleAnimation->setEndValue(0);
    part = spanAngleAnimation->endValue().toReal() / spanAngleAnimation->startValue().toReal();
    spanAngleAnimation->setDuration(std::max(0, (int)(150 * (1 - part))));
    group->addAnimation(spanAngleAnimation);

    auto *topLineAnimation = new QPropertyAnimation(this, "m_topLinePercent", this);
    topLineAnimation->setStartValue(topLinePercent());
    topLineAnimation->setEndValue(0);
    part = topLineAnimation->endValue().toReal() / topLineAnimation->startValue().toReal();
    topLineAnimation->setDuration(std::max(0, (int)(100 * (1 - part))));
    group->addAnimation(topLineAnimation);

    connect(this, &MyLineEdit::focused, group, &QSequentialAnimationGroup::deleteLater);
    connect(group, &QSequentialAnimationGroup::finished, group, &QSequentialAnimationGroup::deleteLater);
    group->start();

    QLineEdit::focusOutEvent(event);
    emit unfocused();
}

void MyLineEdit::set_bottomLinePercent(qreal bottomLine) {
    m_bottomLinePercent = bottomLine;
    emit objectChanged();
}

qreal MyLineEdit::bottomLinePercent() const {
    return m_bottomLinePercent;
}

qreal MyLineEdit::topLinePercent() const {
    return m_topLinePercent;
}

void MyLineEdit::set_topLinePercent(qreal topLine) {
    m_topLinePercent = topLine;
    emit objectChanged();
}

int MyLineEdit::spanAngle() const {
    return m_spanAngle;
}

void MyLineEdit::set_spanAngle(int spanAngle) {
    m_spanAngle = spanAngle;
    emit objectChanged();
}

qreal MyLineEdit::needWidth() const {
    return (QWidget::width() - 20) / 2.0;
}

void MyLineEdit::keyPressEvent(QKeyEvent *event) {
    QLineEdit::keyPressEvent(event);
    if (event->key() == Qt::Key_Escape) {
        clearFocus();
    }
}
