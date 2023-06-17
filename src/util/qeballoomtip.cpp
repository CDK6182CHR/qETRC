#include "qeballoomtip.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QGridLayout>
//#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <QBitmap>

namespace qeutil{

#if 0
static QBalloonTip *theSolitaryBalloonTip = 0;
void QBalloonTip::showBalloon(const QIcon &icon, const QString &title,
                              const QString &message,
                              const QPoint &pos, int timeout, bool showArrow)
{
    hideBalloon();
    if (message.isEmpty() && title.isEmpty())
        return;
    theSolitaryBalloonTip = new QBalloonTip(icon, title, message);
    if (timeout < 0)
        timeout = 10000; //10 s default
    theSolitaryBalloonTip->balloon(pos, timeout, showArrow);
}
void QBalloonTip::hideBalloon()
{
    if (!theSolitaryBalloonTip)
        return;
    theSolitaryBalloonTip->hide();
    delete theSolitaryBalloonTip;
    theSolitaryBalloonTip = 0;
}
void QBalloonTip::updateBalloonPosition(const QPoint& pos)
{
    if (!theSolitaryBalloonTip)
        return;
    theSolitaryBalloonTip->hide();
    theSolitaryBalloonTip->balloon(pos, 0, theSolitaryBalloonTip->showArrow);
}
bool QBalloonTip::isBalloonVisible()
{
    return theSolitaryBalloonTip;
}
#endif

QEBalloonTip::QEBalloonTip(const QIcon& icon, const QString& title,
    const QString& message, QWidget* parent)
    : QWidget(parent, Qt::ToolTip),
//      trayIcon(ti),
      timerId(-1),
      showArrow(true)
{
    //setAttribute(Qt::WA_DeleteOnClose);
#if QT_CONFIG(label)
    titleLabel = new QLabel;
    titleLabel->installEventFilter(this);
    titleLabel->setText(title);
    QFont f = titleLabel->font();
    f.setBold(true);
    titleLabel->setFont(f);
    titleLabel->setTextFormat(Qt::PlainText); // to maintain compat with windows
#endif
    const int iconSize = 18;
    const int closeButtonSize = 15;
#if QT_CONFIG(pushbutton)
    QPushButton *closeButton = new QPushButton;
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setIconSize(QSize(closeButtonSize, closeButtonSize));
    closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setFixedSize(closeButtonSize, closeButtonSize);
    QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
#else
    Q_UNUSED(closeButtonSize);
#endif
#if QT_CONFIG(label)
    msgLabel = new QLabel;
    msgLabel->installEventFilter(this);
    msgLabel->setText(message);
    msgLabel->setTextFormat(Qt::PlainText);
    msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // smart size for the message label
#if 0
    int limit = QDesktopWidgetPrivate::availableGeometry(msgLabel).size().width() / 3;
    if (msgLabel->sizeHint().width() > limit) {
        msgLabel->setWordWrap(true);
        if (msgLabel->sizeHint().width() > limit) {
            msgLabel->d_func()->ensureTextControl();
            if (QWidgetTextControl *control = msgLabel->d_func()->control) {
                QTextOption opt = control->document()->defaultTextOption();
                opt.setWrapMode(QTextOption::WrapAnywhere);
                control->document()->setDefaultTextOption(opt);
            }
        }
        // Here we allow the text being much smaller than the balloon widget
        // to emulate the weird standard windows behavior.
        msgLabel->setFixedSize(limit, msgLabel->heightForWidth(limit));
    }
#endif
#endif
    QGridLayout *layout = new QGridLayout;
#if QT_CONFIG(label)
    if (!icon.isNull()) {
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(icon.pixmap(iconSize, iconSize));
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        iconLabel->setMargin(2);
        layout->addWidget(iconLabel, 0, 0);
        layout->addWidget(titleLabel, 0, 1);
    } else {
        layout->addWidget(titleLabel, 0, 0, 1, 2);
    }
#endif
#if QT_CONFIG(pushbutton)
    layout->addWidget(closeButton, 0, 2);
#endif
#if QT_CONFIG(label)
    layout->addWidget(msgLabel, 1, 0, 1, 3);
#endif
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(3, 3, 3, 3);
    setLayout(layout);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
    pal.setColor(QPalette::WindowText, Qt::black);
    setPalette(pal);
}

void QEBalloonTip::paintEvent(QPaintEvent * e)
{
    Q_UNUSED(e)
    QPainter painter(this);
    painter.drawPixmap(rect(), pixmap);
}
void QEBalloonTip::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);
}



void QEBalloonTip::balloon(const QPoint& pos, int msecs, bool showArrow)
{
    this->showArrow = showArrow;
    QRect scr = QApplication::screenAt(pos)->availableGeometry();
    QSize sh = sizeHint();
    const int border = 1;
    const int ah = 18, ao = 18, aw = 18, rc = 7;
    bool arrowAtTop = (pos.y() + sh.height() + ah < scr.height());
    bool arrowAtLeft = (pos.x() + sh.width() - ao < scr.width());
    setContentsMargins(border + 3,  border + (arrowAtTop ? ah : 0) + 2, border + 3, border + (arrowAtTop ? 0 : ah) + 2);
    updateGeometry();
    sh  = sizeHint();
    int ml, mr, mt, mb;
    QSize sz = sizeHint();
    if (!arrowAtTop) {
        ml = mt = 0;
        mr = sz.width() - 1;
        mb = sz.height() - ah - 1;
    } else {
        ml = 0;
        mt = ah;
        mr = sz.width() - 1;
        mb = sz.height() - 1;
    }
    QPainterPath path;
#if defined(QT_NO_XSHAPE) && 0 /* Used to be included in Qt4 for Q_WS_X11 */
    // XShape is required for setting the mask, so we just
    // draw an ugly square when its not available
    path.moveTo(0, 0);
    path.lineTo(sz.width() - 1, 0);
    path.lineTo(sz.width() - 1, sz.height() - 1);
    path.lineTo(0, sz.height() - 1);
    path.lineTo(0, 0);
    move(qMax(pos.x() - sz.width(), scr.left()), pos.y());
#else
    path.moveTo(ml + rc, mt);
    if (arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ml + ao, mt);
            path.lineTo(ml + ao, mt - ah);
            path.lineTo(ml + ao + aw, mt);
        }
        move(qMax(pos.x() - ao, scr.left() + 2), pos.y());
    } else if (arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao - aw, mt);
            path.lineTo(mr - ao, mt - ah);
            path.lineTo(mr - ao, mt);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y());
    }
    path.lineTo(mr - rc, mt);
    path.arcTo(QRect(mr - rc*2, mt, rc*2, rc*2), 90, -90);
    path.lineTo(mr, mb - rc);
    path.arcTo(QRect(mr - rc*2, mb - rc*2, rc*2, rc*2), 0, -90);
    if (!arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao, mb);
            path.lineTo(mr - ao, mb + ah);
            path.lineTo(mr - ao - aw, mb);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2),
             pos.y() - sh.height());
    } else if (!arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ao + aw, mb);
            path.lineTo(ao, mb + ah);
            path.lineTo(ao, mb);
        }
        move(qMax(pos.x() - ao, scr.x() + 2), pos.y() - sh.height());
    }
    path.lineTo(ml + rc, mb);
    path.arcTo(QRect(ml, mb - rc*2, rc*2, rc*2), -90, -90);
    path.lineTo(ml, mt + rc);
    path.arcTo(QRect(ml, mt, rc*2, rc*2), 180, -90);
    // Set the mask
    QBitmap bitmap = QBitmap(sizeHint());
    bitmap.fill(Qt::color0);
    QPainter painter1(&bitmap);
    painter1.setPen(QPen(Qt::color1, border));
    painter1.setBrush(QBrush(Qt::color1));
    painter1.drawPath(path);
    setMask(bitmap);
#endif
    // Draw the border
    pixmap = QPixmap(sz);
    QPainter painter2(&pixmap);
    painter2.setPen(QPen(palette().color(QPalette::Window).darker(160), border));
    painter2.setBrush(palette().color(QPalette::Window));
    painter2.drawPath(path);
    if (msecs > 0)
        timerId = startTimer(msecs);
    show();
}
void QEBalloonTip::setContents(const QString& title, const QString& msg)
{
    titleLabel->setText(title);
    msgLabel->setText(msg);
}
void QEBalloonTip::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    close();
//    if(e->button() == Qt::LeftButton)
//        emit trayIcon->messageClicked();
}
void QEBalloonTip::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timerId) {
        killTimer(timerId);
        if (!underMouse())
            close();
        return;
    }
    QWidget::timerEvent(e);
}

}
