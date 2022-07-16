#ifndef CLIENT_MYBUTTON_H
#define CLIENT_MYBUTTON_H

#include <QPushButton>

class QGraphicsOpacityEffect;

class MyButton : public QPushButton {
Q_OBJECT
    Q_PROPERTY(qreal opacity READ windowOpacity WRITE setWindowOpacity)
public:
    explicit MyButton(const QString &text, QWidget *parent = nullptr);

    explicit MyButton(QWidget *parent = nullptr);

    void set_bgColor(const QString &bgColor);

    QString bgColor();

    void set_textColor(const QString &textColor);

    QString textColor();

protected:
    void paintEvent(QPaintEvent *) override;

    void enterEvent(QEnterEvent *) override;

    void leaveEvent(QEvent *) override;

    void keyPressEvent(QKeyEvent *) override;

private slots:

    void buttonPressed();

    void buttonReleased();

    void showClickAnimation();

    void showReleaseAnimation();

private:
    QString m_textColor;
    QString m_bgColor;
    bool m_pressed{};
    bool m_hovered{};

    QGraphicsOpacityEffect *m_effect;
};

#endif //CLIENT_MYBUTTON_H
