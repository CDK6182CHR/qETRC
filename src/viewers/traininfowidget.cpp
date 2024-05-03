#include "traininfowidget.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QScroller>

#include "data/train/train.h"
#include "data/train/routing.h"
#include "data/analysis/runstat/trainintervalstat.h"
#include "data/train/traintype.h"
#include "util/utilfunc.h"
#include "util/dialogadapter.h"
#include "defines/icon_specs.h"

TrainInfoWidget::TrainInfoWidget(QWidget *parent) : QScrollArea(parent)
{
    setWindowTitle(tr("速览信息"));
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setWidgetResizable(true);
    initUI();
    QScroller::grabGesture(this, QScroller::TouchGesture);
}

void TrainInfoWidget::initUI()
{
    auto* w = new QWidget;
    auto* vlay=new QVBoxLayout(w);
    flay=new QFormLayout;

    edName=makeLineEdit(tr("全车次"));
    edNameDir=makeLineEdit(tr("分方向车次"));
    edStartEnd=makeLineEdit(tr("始发终到"));
    edType=makeLineEdit(tr("列车种类"));
    edPassen=makeLineEdit(tr("旅客列车"));
    edStations=makeLineEdit(tr("总/铺画站数"));
    edLines=makeLineEdit(tr("运行线情况"));

    edTotTime = makeLineEdit(tr("总历时"));
    edTotRun = makeLineEdit(tr("总运行时间"));
    edTotStay = makeLineEdit(tr("总停站时间"));
    edTotMile = makeLineEdit(tr("总里程"));
    edTotSpeed = makeLineEdit(tr("总旅行速度"));
    edTotTechSpeed = makeLineEdit(tr("总技术速度"));

    edMile=makeLineEdit(tr("铺画里程"));
    edTime=makeLineEdit(tr("铺画总时间"));
    edTime->setToolTip(tr("本次列车在本运行图所有线路中的所有运行线的总时长"));
    //edRun=makeLineEdit(tr("铺画运行时间"));
    //edStay=makeLineEdit(tr("铺画停站时间"));
    edSpeed=makeLineEdit(tr("铺画旅行速度"));
    edTechSpeed=makeLineEdit(tr("铺画技术速度"));

    edRouting = new QLineEdit;
    edRouting->setFocusPolicy(Qt::NoFocus);
    auto* hlay = new QHBoxLayout;
    hlay->addWidget(edRouting);
    auto* tb = new QToolButton(this);
    tb->setIcon(QEICN_train_info_to_routing);
    connect(tb, &QToolButton::clicked, this, &TrainInfoWidget::actSwitchToRouting);
    tb->setToolTip(tr("转到当前交路\n将当前列车所属的交路（如果存在）设置为“当前交路”，"
        "可切换到工具栏的交路上下文页面对交路操作。"));
    hlay->addWidget(tb);
    flay->addRow(tr("所属交路"), hlay);
    
    edModel=makeLineEdit(tr("车底型号"));
    edOwner=makeLineEdit(tr("担当交路"));
    edPre=makeLineEdit(tr("前序 (连线)"));
    edPost=makeLineEdit(tr("后序 (连线)"));

    auto* btn=new QPushButton(tr("点击查看"));
    flay->addRow(tr("时刻表"),btn);
    btn->setMaximumWidth(150);
    connect(btn,&QPushButton::clicked,this,&TrainInfoWidget::actShowTimetable);

    hlay = new QHBoxLayout;
    btn = new QPushButton(tr("时刻"));
    hlay->addWidget(btn);
    btn->setMaximumWidth(60);
    connect(btn,&QPushButton::clicked,this,&TrainInfoWidget::actEditTimetable);

    btn = new QPushButton(tr("全部"));
    hlay->addWidget(btn);
    btn->setMaximumWidth(60);
    connect(btn, &QPushButton::clicked, this, &TrainInfoWidget::actEditTrain);

    flay->addRow(tr("编辑"), hlay);

    vlay->addLayout(flay);
    vlay->addWidget(new QLabel(tr("交路序列")));
    edOrder=new QTextBrowser;
    vlay->addWidget(edOrder);

    btn=new QPushButton(tr("导出为文本"));
    connect(btn,&QPushButton::clicked,this,&TrainInfoWidget::toText);
    vlay->addWidget(btn);
    setWidget(w);
}

QLineEdit *TrainInfoWidget::makeLineEdit(const QString &title) const
{
    auto* ed=new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    ed->setReadOnly(true);
    ed->setAttribute(Qt::WA_InputMethodEnabled,false);
    flay->addRow(title,ed);
    return ed;
}

void TrainInfoWidget::setTrain(std::shared_ptr<Train> train)
{
    this->train=train;
    refreshData();
}

void TrainInfoWidget::resetTrain()
{
    this->train = nullptr;
    clearData();
}

void TrainInfoWidget::refreshData()
{
    if (!train) {
        clearData();
        return;
    }
    edName->setText(train->trainName().full());
    edNameDir->setText(tr("%1 / %2").arg(train->trainName().down(),
                                         train->trainName().up()));
    edStartEnd->setText(tr("%1 -> %2").arg(train->starting().toSingleLiteral(),
                                           train->terminal().toSingleLiteral()));
    edType->setText(train->type()->name());

    if(train->passenger() == TrainPassenger::Auto){
        edPassen->setText(tr("自动 (%1)").arg(
                            train->getIsPassenger()?tr("是"):tr("否")  ));
    }else{
        edPassen->setText(tr("指定 (%1)").arg(
                            train->passenger() == TrainPassenger::True?tr("是"):tr("否")));
    }
    edStations->setText(tr("%1 / %2").arg(train->stationCount())
                        .arg(train->adapterStationCount()));
    edLines->setText(tr("%1线路 | %2运行线").arg(train->adapters().size())
                     .arg(train->lineCount()));

    // for global information
    TrainIntervalStatResult res{};
    
    if (train->empty()) {
        // use zero-initialized res    
    }
    else {
        TrainIntervalStat stat{ train };
        stat.setIncludeEnds(true);   // 2024.05.03
        stat.setRange(0, train->stationCount() - 1);
        res = stat.compute();
    }
    edTotTime->setText(qeutil::secsToStringHour(res.totalSecs));
    edTotRun->setText(qeutil::secsToStringHour(res.runSecs));
    edTotStay->setText(qeutil::secsToStringHour(res.stopSecs));
    if (res.railResults.isValid) {
        edTotMile->setText(tr("%1 km").arg(QString::number(res.railResults.totalMiles, 'f', 3)));
        edTotSpeed->setText(tr("%1 km/h").arg(QString::number(res.railResults.travelSpeed, 'f', 3)));
        edTotTechSpeed->setText(tr("%1 km/h").arg(QString::number(res.railResults.techSpeed, 'f', 3)));
    }
    else {
        edTotMile->setText("NA");
        edTotSpeed->setText("NA");
        edTotTechSpeed->setText("NA");
    }

    double mile=train->localMile();
    auto [run,stay]=train->localRunStaySecs();

    edMile->setText(tr("%1 km").arg(QString::number(mile,'f',3)));
    edTime->setText(qeutil::secsToStringHour(run+stay));
    //edRun->setText(qeutil::secsToStringHour(run));
    //edStay->setText(qeutil::secsToStringHour(stay));

    double spd = mile / (run + stay) * 3600;
    double techspd = mile / run * 3600;

    edSpeed->setText(tr("%1 km/h").arg(QString::number(spd, 'f', 3)));
    edTechSpeed->setText(tr("%1 km/h").arg(QString::number(techspd, 'f', 3)));

    if (train->hasRouting()) {
        auto rt = train->routing().lock();
        edRouting->setText(rt->name());
        edModel->setText(rt->model());
        edOwner->setText(rt->owner());
        edPre->setText(rt->preOrderString(*train));
        edPost->setText(rt->postOrderString(*train));
        edOrder->setText(rt->orderString());
    }
    else {
        edRouting->setText(tr("(无交路信息)"));
        edModel->clear();
        edOwner->clear();
        edPre->clear();
        edPost->clear();
        edOrder->clear();
    }
}

void TrainInfoWidget::clearData()
{
    // For the lineEdits
    std::initializer_list<QLineEdit*> lineEdits{
        edName, edNameDir, edStartEnd, edType, edPassen,
        edStations, edLines, edMile, edTime, edSpeed,
        edTechSpeed, edRouting, edModel, edOwner, edPre, edPost,
        edTotTime,edTotRun, edTotStay, edTotMile, edTotSpeed, edTotTechSpeed
    };
    for (auto* p : lineEdits) {
        p->clear();
    }

    // Other widgets
    edOrder->clear();
}

void TrainInfoWidget::toText()
{
    if(!train) return;
    QString text;
    text.append(tr("车次：%1\n").arg(train->trainName().full()));
    text.append(tr("分方向车次：%1 / %2\n").arg(train->trainName().down(),train->trainName().up()));
    text.append(tr("始发终到：%1 -> %2\n").arg(train->starting().toSingleLiteral(),
                                        train->terminal().toSingleLiteral()));
    text.append(tr("列车种类：%1\n").arg(train->type()->name()));
    if(train->passenger() == TrainPassenger::Auto){
        text.append(tr("客车类型：自动 (%1)\n").arg(
                            train->getIsPassenger()?tr("是"):tr("否")  ));
    }else{
        text.append(tr("客车类型：指定 (%1)\n").arg(
                            train->passenger() == TrainPassenger::True?tr("是"):tr("否")));
    }

    text.append(tr("总(铺画)站数：%1 / %2\n").arg(train->stationCount())
                        .arg(train->adapterStationCount()));
    text.append(tr("运行线信息：%1线路 | %2运行线\n").arg(train->adapters().size())
                     .arg(train->lineCount()));

    // global information, just use that in the editors to avoid overhead
    text.append(tr("总时间：%1\n").arg(edTotTime->text()));
    text.append(tr("总运行时间：%1\n总停站时间：%2\n").arg(edTotRun->text(), edTotStay->text()));
    text.append(tr("总里程：%1\n总旅行速度：%2\n总技术速度：%3\n").arg(edTotMile->text(),
        edTotSpeed->text(), edTotTechSpeed->text()));

    double mile=train->localMile();
    auto [run,stay]=train->localRunStaySecs();

    text.append(tr("铺画里程：%1 km\n").arg(QString::number(mile,'f',3)));
    text.append(tr("铺画运行时间：") + qeutil::secsToStringHour(run+stay)+'\n');
    text.append(tr("铺画纯运行时间：") + qeutil::secsToStringHour(run)+'\n');
    text.append(tr("图定总停站时间：") + qeutil::secsToStringHour(stay)+'\n');

    double spd = mile / (run + stay) * 3600;
    double techspd = mile / run * 3600;

    text.append(tr("铺画旅行速度：%1 km/h\n").arg(QString::number(spd, 'f', 3)));
    text.append(tr("铺画技术速度：%1 km/h\n").arg(QString::number(techspd, 'f', 3)));

    if (train->hasRouting()) {
        auto rt = train->routing().lock();
        text.append(tr("车底交路：") + rt->name()+'\n');
        text.append(tr("车底类型：") + rt->model()+'\n');
        text.append(tr("担当局段：") + rt->owner()+'\n');
        // todo 前序后序
        text.append(tr("前序（连线）：") + rt->preOrderString(*train)+'\n');
        text.append(tr("后续（连线）：") + rt->postOrderString(*train)+'\n');
        text.append(tr("交路序列：") + rt->orderString()+'\n');
    }
    else {
        text.append(tr("本次列车无交路信息\n"));
    }

    auto* t=new QTextBrowser();
    t->setWindowTitle(tr("列车信息导出"));
    t->setText(text);
    auto* dialog=new DialogAdapter(t);
    dialog->resize(400,400);
    dialog->show();
}

void TrainInfoWidget::actEditTimetable()
{
    if (train)
        emit editTimetable(train);
}

void TrainInfoWidget::actEditTrain()
{
    if(train)
        emit editTrain(train);
}

void TrainInfoWidget::actShowTimetable()
{
    if(train)
        emit showTimetable(train);
}

void TrainInfoWidget::actSwitchToRouting()
{
    if (!train)return;
    if (auto rt = train->routing().lock()) {
        emit switchToRouting(rt);
    }
    else {
        QMessageBox::warning(this, tr("错误"), tr("当前车次无交路信息，无法转到对应交路。"));
    }
}
