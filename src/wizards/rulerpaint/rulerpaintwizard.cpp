#include "rulerpaintwizard.h"

#include "data/diagram/diagram.h"
#include "data/diagram/trainadapter.h"
#include "rulerpaintpagestart.h"
#include "rulerpaintpagestation.h"
#include "rulerpaintpagetable.h"
#include "data/rail/railway.h"

#include <QMessageBox>

RulerPaintWizard::RulerPaintWizard(Diagram &diagram_, QWidget *parent):
    QWizard(parent), diagram(diagram_)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("标尺排图向导"));
    resize(800,800);
    initUI();
}

void RulerPaintWizard::reject()
{
    if (visitedIds().contains(1)) {
        auto flag = QMessageBox::question(this, tr("标尺排图向导"),
            tr("是否确认退出标尺排图向导？未提交的更改将丢失。"));
        if (flag != QMessageBox::Yes)
            return;    //打断施法
    }

    if (trainTmp)
        emit removeTmpTrainLine(*trainTmp);
    
    QWizard::reject();
}

void RulerPaintWizard::accept()
{
    if (trainTmp)
        emit removeTmpTrainLine(*trainTmp);
    else {
        QMessageBox::warning(this, tr("错误"), tr("未铺画列车运行图。请在排图表格中，"
            "双击指定行排图，或者更改排图范围选项。"));
        return;
    }
    auto m = pgStart->getMode();
    if (m == RulerPaintPageStart::NewTrain) {
        trainTmp->setOnPainting(false);
        emit trainAdded(trainTmp);
    }
    else {
        if (trainTmp->starting() != trainRef->starting() ||
            trainTmp->terminal() != trainRef->terminal()) {
            //始发或者终到站更改，先执行列车信息变化
            emit trainInfoModified(pgStart->train(), trainTmp);
        }
        //执行时刻表变化
        emit trainTimetableModified(pgStart->train(), trainTmp);
    }
    QWizard::accept();
}

void RulerPaintWizard::initializePage(int id)
{
    if (id == 1) {
        //第一页结束时，收下当前列车等信息
        trainRef = pgStart->train();
    }
    QWizard::initializePage(id);
    if (id == 1) {
        setDefaultAnchorStation();
    }
    else if (id == 2) {
        //table那一页，尽可能利用一下既有信息
        auto m = pgStart->getMode();
        auto st = pgStation->anchorStation();
        Train::ConstStationPtr itr;
        if (m == RulerPaintPageStart::Prepend) {
            //前缀：如果锚点车站是既有时刻表的第一站
            if (!trainRef->empty() && (itr=trainRef->firstStation())->name == st->name) {
                pgTable->setAnchorTime(itr->arrive, itr->depart);
            }
        }
        else if (m == RulerPaintPageStart::Append) {
            // 后缀：如果锚点车站是既有时刻表的最后一站
            if (!trainRef->empty() && (itr = trainRef->lastStation())->name == st->name) {
                pgTable->setAnchorTime(itr->arrive, itr->depart);
            }
        }
        else if (m == RulerPaintPageStart::Modify) {
            // 调整：如果锚点车站是被覆盖的任意一站
            auto p = trainRef->timetable().cbegin();
            std::advance(p, pgStart->startRow());
            int cnt = pgStart->endRow() - pgStart->startRow() + 1;
            for (int i = 0; i < cnt; i++, ++p) {
                if (p->name == st->name) {
                    pgTable->setAnchorTime(p->arrive, p->depart);
                    break;
                }
            }
        }
        pgTable->setRefTrain(pgStart->train());
    }
}

void RulerPaintWizard::cleanupPage(int id)
{
    if (id == 2) {
        //清理临时运行线
        if (trainTmp)
            emit removeTmpTrainLine(*trainTmp);
    }
    QWizard::cleanupPage(id);
}

void RulerPaintWizard::initUI()
{
    pgStart=new RulerPaintPageStart(diagram.trainCollection());
    addPage(pgStart);
    pgStation = new RulerPaintPageStation(diagram.railCategory());
    connect(pgStation, &RulerPaintPageStation::railwayChanged,
        this, &RulerPaintWizard::setDefaultAnchorStation);
    addPage(pgStation);
    pgTable=new RulerPaintPageTable(diagram, pgStation);
    connect(pgTable->getModel(), &RulerPaintModel::updateTrainLine,
        this, &RulerPaintWizard::updateTrainLine);
    addPage(pgTable);
}

void RulerPaintWizard::resetTmpTrain()
{
    trainTmp = std::make_shared<Train>(*trainRef);    //copy construct
    trainTmp->setOnPainting(true);
    trainTmp->setType(diagram.trainCollection().typeManager().fromRegex(trainTmp->trainName()));
    if (pgStart->getMode() == RulerPaintPageStart::Modify) {
        itrStart = trainTmp->timetable().begin();
        std::advance(itrStart, pgStart->startRow());
        itrEnd = trainTmp->timetable().begin();
        std::advance(itrEnd, pgStart->endRow());
    }
    else {
        itrStart = itrEnd = trainTmp->timetable().end();
    }
}

void RulerPaintWizard::updateTrainLine(std::shared_ptr<Train> table)
{
    if (trainTmp)
        emit removeTmpTrainLine(*trainTmp);
    resetTmpTrain();
    table->autoBusinessWithoutBound(*pgStation->railway(), trainTmp->getIsPassenger());

    //下面：整合算法
    auto m = pgStart->getMode();
    if (m == RulerPaintPageStart::NewTrain) {
        //新车次：直接把时刻表拖过去就完了
        trainTmp->timetable() = std::move(table->timetable());
    }
    else if (m == RulerPaintPageStart::Prepend) {
        //前缀
        if (!trainTmp->empty() && !table->empty()) {
            auto t1 = trainTmp->timetable().begin();
            auto t2 = table->lastStation();
            if (t1->name == t2->name) {
                trainTmp->timetable().erase(t1);
            }
        }
        trainTmp->timetable().splice(trainTmp->timetable().begin(),
            std::move(table->timetable()));
    }
    else if (m == RulerPaintPageStart::Append) {
        //后缀
        if (!trainTmp->empty() && !table->empty()) {
            auto t1 = trainTmp->lastStation();
            auto t2 = table->firstStation();
            if (t1->name == t2->name) {
                trainTmp->timetable().erase(t1);
            }
        }
        trainTmp->timetable().splice(trainTmp->timetable().end(),
            std::move(table->timetable()));
    }
    else {
        //区间时刻重排  注意train的接口要求两个车次都非空
        if (table->empty()) {
            //nothing todo
        }
        else if (trainTmp->empty()) {
            //直接把那边的表搞过来就行了
            trainTmp->timetable() = std::move(table->timetable());
        }
        else {
            //双非空，使用区间换线的接口
            trainTmp->intervalExchange(*table, itrStart, itrEnd,
                table->timetable().begin(), std::prev(table->timetable().end()), true, true);
        }
    }

    //始发终到
    if (!table->starting().empty())
        trainTmp->setStarting(table->starting());
    if (!table->terminal().empty())
        trainTmp->setTerminal(table->terminal());

    //绑定、铺画
    diagram.updateTrain(trainTmp);
    emit paintTmpTrainLine(*trainTmp);
    
}

void RulerPaintWizard::setDefaultAnchorStation()
{
    auto train=pgStart->train();
    if(!train)
        return;
    auto adp = train->adapterFor(*(pgStation->railway()));
    
    if (adp && !adp->isNull()) {
        auto m = pgStart->getMode();
        if (m == RulerPaintPageStart::Modify) {
            //注意不能用itrStart，那是tmpTrain里面的迭代器
            auto _itr = train->timetable().begin(); std::advance(_itr, pgStart->startRow());
            auto p = adp->stationByTrainLinear(_itr);
            if (p.first) {
                pgStation->setDefaultAnchor(p.second->dir(), p.first->railStation.lock());
            }
        }
        else {
            switch (m)
            {
            case RulerPaintPageStart::Prepend:
                pgStation->setDefaultAnchor(adp->firstDirection(),
                    adp->firstStation()->railStation.lock());
                break;
            case RulerPaintPageStart::Append:
                pgStation->setDefaultAnchor(adp->lastDirection(),
                    adp->lastStation()->railStation.lock());
                break;
            default:
                break;
            }
        }
    }
}
