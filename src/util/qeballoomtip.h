#pragma once

#include <QWidget>

class QLabel;

namespace qeutil{

/**
 * @brief The QEBalloonTip class
 * 复制自Qt源码
 * 取消SystemTrayIcon的引用；取消全局单例规定；
 * 新增设置msg的接口
 */
class QEBalloonTip : public QWidget
{
    Q_OBJECT
public:
    //static void showBalloon(const QIcon &icon, const QString &title,
    //                        const QString &msg,
    //                        const QPoint &pos, int timeout, bool showArrow = true);
    //static void hideBalloon();
    //static bool isBalloonVisible();
    //static void updateBalloonPosition(const QPoint& pos);

public:
    QEBalloonTip(const QIcon &icon, const QString &title,
                const QString &msg, QWidget* parent=nullptr);
    //~QEBalloonTip();
    void balloon(const QPoint& pos, int msecs, bool arrow);
    void setContents(const QString& title, const QString& msg);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void timerEvent(QTimerEvent *e) override;

private:
    QPixmap pixmap;
    int timerId;
    bool showArrow;
    QLabel* titleLabel, * msgLabel;
};
}
