#include "MyButton.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QPainterPath>

MyButton::MyButton(const QString &text, QWidget *parent) : QPushButton(text, parent) {
    connect(this, &QPushButton::pressed, this, &MyButton::showClickAnimation);
    connect(this, &QPushButton::pressed, this, &MyButton::buttonPressed);
    connect(this, &QPushButton::released, this, &MyButton::showReleaseAnimation);
    connect(this, &QPushButton::released, this, &MyButton::buttonReleased);
    setCursor(Qt::PointingHandCursor);

    m_bgColor = "#777";
    m_textColor = "black";
    m_effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(m_effect);
    m_effect->setOpacity(1);
}

MyButton::MyButton(QWidget *parent) : MyButton("", parent) {
}

void MyButton::paintEvent(QPaintEvent *event) {
    auto p = new QPainter(this);
    p->setRenderHint(QPainter::Antialiasing);

    p->setPen(Qt::NoPen);
    auto rect = new QRect(0, 0, QWidget::width(), QWidget::height());

    // draw bg
    p->setBrush(QColor(bgColor()));
    QRectF curRect(0.5, 1, rect->width() - 0.5, rect->height() - 1.5);
    p->drawRoundedRect(curRect, QWidget::height() / 2., QWidget::height() / 2.);

    QPainterPath path;
    path.addRoundedRect(curRect, QWidget::height() / 2., QWidget::height() / 2.);
    if (hasFocus()) {
        p->setPen("black");
    } else {
        p->setPen("#555");
    }
    p->drawPath(path);

    if (m_hovered) {
        m_effect->setOpacity(0.6);
    } else {
        m_effect->setOpacity(1);
    }

    // draw text
    p->setPen(QPen(textColor()));
    int width = fontMetrics().horizontalAdvance(text());
    p->drawText(rect->width() / 2 - width / 2, 18, text());

    p->end();
}

void MyButton::set_bgColor(const QString &bgColor) {
    m_bgColor = bgColor;
}

QString MyButton::bgColor() {
    return m_bgColor;
}

void MyButton::set_textColor(const QString &textColor) {
    m_textColor = textColor;
}

QString MyButton::textColor() {
    return m_textColor;
}

void MyButton::buttonPressed() {
    m_pressed = true;
    update();
}

void MyButton::buttonReleased() {
    m_pressed = false;
    update();
}

void MyButton::showClickAnimation() {
    auto *animation = new QPropertyAnimation(m_effect, "opacity", this);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    animation->setDuration(500);
    animation->setStartValue(m_effect->opacity());
    animation->setEndValue(0.5);
    animation->start();
}

void MyButton::showReleaseAnimation() {
    auto *animation = new QPropertyAnimation(m_effect, "opacity", this);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    animation->setDuration(500);
    animation->setStartValue(m_effect->opacity());
    animation->setEndValue(1);
    animation->start();
}

void MyButton::enterEvent(QEnterEvent *event) {
    m_hovered = true;
    update();
}

void MyButton::leaveEvent(QEvent *event) {
    m_hovered = false;
    update();
}

void MyButton::keyPressEvent(QKeyEvent *event) {
    QPushButton::keyPressEvent(event);
}
