#ifndef CLIENT_MYLINEEDIT_H
#define CLIENT_MYLINEEDIT_H

#include <QLineEdit>
#include <QSequentialAnimationGroup>

class MyLineEdit : public QLineEdit {
Q_OBJECT
    Q_PROPERTY(qreal m_bottomLinePercent READ bottomLinePercent WRITE set_bottomLinePercent)
    Q_PROPERTY(qreal m_topLinePercent READ topLinePercent WRITE set_topLinePercent)
    Q_PROPERTY(int m_spanAngle READ spanAngle WRITE set_spanAngle)
public:
    explicit MyLineEdit(QWidget *parent = nullptr);

    explicit MyLineEdit(const QString &text, QWidget *parent = nullptr);

    [[nodiscard]] qreal bottomLinePercent() const;

    void set_bottomLinePercent(qreal bottomLine);

    [[nodiscard]] qreal topLinePercent() const;

    void set_topLinePercent(qreal topLine);

    [[nodiscard]] int spanAngle() const;

    void set_spanAngle(int spanAngle);

signals:

    void objectChanged();

    void focused();

    void unfocused();

private slots:

    void updater();

protected:
    void paintEvent(QPaintEvent *) override;

    void focusInEvent(QFocusEvent *) override;

    void focusOutEvent(QFocusEvent *) override;

    void keyPressEvent(QKeyEvent *) override;

private:
    qreal m_bottomLinePercent;
    qreal m_topLinePercent;
    int m_spanAngle;

    [[nodiscard]] qreal needWidth() const;
};

#endif //CLIENT_MYLINEEDIT_H
