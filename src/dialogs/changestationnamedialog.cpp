#ifndef QETRC_MOBILE_2

#include "changestationnamedialog.h"

#include <QtWidgets>
#include "data/diagram/diagram.h"
#include "data/rail/rail.h"

#include "mainwindow/mainwindow.h"

ChangeStationNameDialog::ChangeStationNameDialog(Diagram &diagram_, QWidget *parent):
    QDialog(parent),diagram(diagram_)
{
    setWindowTitle(tr("全局站名修改"));
    resize(400,300);
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
}

void ChangeStationNameDialog::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("此功能将在整个运行图文件的范围内，将出现的原站名修改为新站名，"
        "然后刷新运行图。"
        "修改的范围包括线路站名（含标尺、天窗等数据）、列车时刻表中的站名和列车始发、终到站的站名。"
        "请注意不允许将线路中的一个站名修改为另一个站名。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* flay=new QFormLayout;
    edOld=new QLineEdit;
    edNew=new QLineEdit;
    flay->addRow(tr("原站名"),edOld);
    flay->addRow(tr("新站名"),edNew);
    vlay->addLayout(flay);

    auto* box=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    connect(box,SIGNAL(accepted()),this,SLOT(onApplyClicked()));
    connect(box,SIGNAL(rejected()),this,SLOT(close()));
    vlay->addWidget(box);
}

void ChangeStationNameDialog::onApplyClicked()
{
    QString sold=edOld->text(),snew=edNew->text();
    if(sold.isEmpty()||snew.isEmpty()||sold==snew){
        QMessageBox::warning(this,tr("错误"),
          tr("原站名、新站名不能为空或相同，请重新填写！"));
        return;
    }
    for(auto r:diagram.railways()){
        auto st=r->stationByName(StationName::fromSingleLiteral(sold));
        if(st && r->stationNameExisted(snew)){
            //注意：这里采用严格匹配，且新的站名已经确定和原来不同，
            //相当于原站对象已经被忽略掉了
            QMessageBox::warning(this,tr("错误"),
                                 tr("站名修改在线路[%1]引起站名冲突，请重新设置。\n"
            "提示：不允许将一条线路中一个站的站名改为另一个站的站名。")
                                 .arg(r->name()));
            return;
        }
    }

    data.oldName = sold;
    data.newName = snew;

    //现在：校验通过，开始设置数据  注意开始之前，数据一定是空的
    for(auto r:diagram.railways()){
        for(auto t:r->stations()){
            if(t->name.toSingleLiteral() == sold){
                data.railStations.append(
                    std::make_tuple(r, t,StationName::fromSingleLiteral(snew)));
            }
        }
    }

    for(auto train:diagram.trains()){
        //始发终到
        if(train->starting().toSingleLiteral()==sold){
            data.startings.append(std::make_pair(train,snew));
        }
        if(train->terminal().toSingleLiteral()==sold){
            data.terminals.append(std::make_pair(train,snew));
        }
        //站名
        for(auto p=train->timetable().begin();p!=train->timetable().end();++p){
            if(p->name.toSingleLiteral() == sold){
                data.trainStations.append(std::make_pair(p, snew));
            }
        }
    }

    //输出一个报告
    bool flag = false;
    QString text = tr("站名更改成功。影响到：");
    if (!data.railStations.isEmpty()) {
        flag = true;
        text += tr("\n%1个线路站名").arg(data.railStations.size());
    }
    if (!data.trainStations.isEmpty()) {
        flag = true;
        text += tr("\n%1个列车时刻表站名").arg(data.trainStations.size());
    }
    if (!data.startings.isEmpty()) {
        flag = true;
        text += tr("\n%1个列车始发站站名").arg(data.startings.size());
    }
    if (!data.terminals.isEmpty()) {
        flag = true;
        text += tr("\n%1个列车终到站站名").arg(data.terminals.size());
    }
    if (flag) {
        //只有有效更改才提交出去
        emit nameChangeApplied(data);
        QMessageBox::information(this, tr("全局站名修改"), text);
        done(QDialog::Accepted);
    }
    else {
        QMessageBox::information(this, tr("全局站名修改"),
            tr("站名修改无任何影响。"));
    }
}

void ChangeStationNameData::commit()
{
    for (auto& p : railStations) {
        std::get<0>(p)->swapStationName(std::get<1>(p), std::get<2>(p));
    }
    for (auto& p : trainStations) {
        std::swap(p.first->name, p.second);
    }
    for (auto& p : startings) {
        std::swap(p.first->startingRef(), p.second);
    }
    for (auto& p : terminals) {
        std::swap(p.first->terminalRef(), p.second);
    }
}

void qecmd::ChangeStationNameGlobal::undo()
{
    data.commit();
    mw->refreshAll();
}

void qecmd::ChangeStationNameGlobal::redo()
{
    data.commit();
    mw->refreshAll();
}

#endif
