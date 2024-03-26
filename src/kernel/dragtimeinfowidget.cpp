#include "dragtimeinfowidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTime>

DragTimeInfoWidget::DragTimeInfoWidget(QWidget* parent):
    QFrame(parent)
{
    initUI();
}

void DragTimeInfoWidget::showInfo(bool shift, const QString &trainName, const QString &stationName,
                                  const QString &pointName, const QTime &oldTime, const QTime &newTime)
{
    QString html_base{};
    if (shift) {
        // shift case
        html_base = QObject::tr(R"(
    平移区段运行线 <br>  
    <font size=5>%1</font> 次 <font size=5>%2</font> 站 <font size=5>%3</font> 时刻
    <br>
    <font size=5>%4</font> → <font size=5 color="blue">%5</font>
)");
    }
    else {
        html_base = QObject::tr(R"(
    调整单站时刻 <br>
    <font size=5>%1</font> 次 <font size=5>%2</font> 站 <font size=5>%3</font> 时刻
    <br>
    <font size=5>%4</font> → <font size=5 color="blue">%5</font>
)");
    }

    auto html=html_base.arg(trainName, stationName, pointName,
        oldTime.toString("hh:mm:ss"), newTime.toString("hh:mm:ss"));

    labMain->setText(html);

    show();
}

void DragTimeInfoWidget::initUI()
{
#if 0
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;

    QFont fontSmall;
    QFont fontLarge;

    fontLarge.setPointSize(static_cast<int>(fontSmall.pointSize()*1.5));

    labTrain=new QLabel;
    labTrain->setFont(fontLarge);
    hlay->addWidget(labTrain, 10);
    auto* lab=new QLabel("次");
    hlay->addWidget(lab);
    hlay->addStretch(1);

    labStation=new QLabel;
    labStation->setFont(fontLarge);
    hlay->addWidget(labStation, 10);

    lab=new QLabel("站");
    hlay->addWidget(lab);
    hlay->addStretch(1);

    labPoint=new QLabel;
    labPoint->setFont(fontLarge);
    hlay->addWidget(labPoint, 4);

    lab=new QLabel("时刻");
    hlay->addWidget(lab);

    vlay->addLayout(hlay);

    hlay=new QHBoxLayout;
    labOldTime=new QLabel;
    labOldTime->setFont(fontLarge);
#endif

    auto* vlay=new QVBoxLayout(this);
    labMain=new QLabel;
    vlay->addWidget(labMain);
    setStyleSheet("background-color: rgba(230,230,230,128);");
}
