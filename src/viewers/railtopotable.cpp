#include "railtopotable.h"

#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>

#include <data/common/qesystem.h>
#include <data/rail/forbid.h>
#include <data/rail/railway.h>
#include <map>


RailTopoTable::RailTopoTable(std::shared_ptr<Railway> rail, QWidget *parent):
    QWidget(parent), railway(rail)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::Dialog);
    initUI();
    refreshData();
}

void RailTopoTable::refreshData()
{
    using TI=QTableWidgetItem;
    setWindowTitle(tr("线路拓扑 - %1").arg(railway->name()));
    table->setRowCount(railway->stationCount()*2);

    // 第一轮循环，从车站角度填信息，并构建序号映射表
    // 这实际上就是Railway::numberMap，不过无所谓了，反正要循环一次。
    std::map<const RailStation*, int> stNums;
    for(int i=0;i<railway->stationCount();i++){
        auto st=railway->stations().at(i);
        stNums.emplace(st.get(), i);

        auto* it=new TI(st->name.toSingleLiteral());
        it->setTextAlignment(Qt::AlignCenter);
        table->setItem(i*2, ColName, it);
        it->setBackground(QColor::fromRgb(245, 245, 245));

        it=new TI(QString::number(st->mile,'f',3));
        it->setTextAlignment(Qt::AlignCenter);
        table->setItem(i*2,ColMile,it);

        it=new TI(st->counterStr());
        it->setTextAlignment(Qt::AlignCenter);
        table->setItem(i*2,ColCounter,it);

        for (int c : {ColName, ColMile, ColCounter}) {
            table->setSpan(2*i,c,2,1);
        }
    }

    int warnCount=0;
    // 第二轮循环走下行 相关警告填在Info的偶数行
    for(auto p=railway->firstDownInterval();p;p=p->nextInterval()){
        auto from=p->fromStation();
        auto to=p->toStation();
        int fromidx=stNums.at(from.get());
        int toidx=stNums.at(to.get());

        // 单线区间的警告可以放到warn2里面。反正对面用不到。
        QString warn;

        QString mile_s=QString::number(p->mile(),'f',3);
        bool isSingle=false;

        // 产生可能的Warning，主要是单线区间才会有。
        if (to->prevSingle){
            if (!p->isSymmetryInterval()){
                warn.append(tr("[%1] 非对称区间无法设置为单线区间  ").arg(++warnCount));
            }else{
                // 本分支是有效的单线区间
                isSingle=true;
                auto conj=p->inverseInterval();
                if (std::abs(p->mile()-conj->mile())>1e-5){
                    warn.append(tr("[%1] 单线区间里程不一致  ").arg(++warnCount));
                    mile_s.append(tr("/%1").arg(
                                      conj->mile(),0,'f',3));
                }
                // 天窗
                for (int i=0;i<Forbid::FORBID_COUNT;i++){
                    auto node_down=p->template getDataAt<ForbidNode>(i);
                    auto node_up=conj->template getDataAt<ForbidNode>(i);
                    if (*node_down!=*node_up){
                        warn.append(tr("[%1] 单线区间天窗不一致 (%2)  ").arg(++warnCount)
                                    .arg(railway->forbids().at(i)->name()));
                    }
                }
            }
        }

        auto* it=new TI(mile_s);
        it->setTextAlignment(Qt::AlignCenter);
        table->setItem(2*fromidx+1, ColDown, it);
        int spanCol=isSingle?2:1;
        table->setSpan(2*fromidx+1, ColDown, (toidx-fromidx)*2, spanCol);

        if (!warn.isEmpty()){
            table->setItem(2*fromidx+1, ColInfo, new TI(warn));
        }
    }

    // 第三轮循环走上行。上行的不用管已经确认的单线区间，但要检查不合理的设置。
    for (auto p=railway->firstUpInterval();p;p=p->nextInterval()){
        auto from=p->fromStation();
        auto to=p->toStation();
        int fromidx=stNums.at(from.get());
        int toidx=stNums.at(to.get());

        QString warn;
        bool isSingle=false;
        if (from->prevSingle){
            if (!p->isSymmetryInterval()){
                warn.append(tr("[%1] 非对称区间无法设置为单线区间").arg(++warnCount));
            }else{
                isSingle=true;
            }
        }

        if (!isSingle){
            auto* it=new TI(QString::number(p->mile(),'f',3));
            it->setTextAlignment(Qt::AlignCenter);
            table->setItem(2*toidx+1, ColUp, it);

            table->setSpan(2*toidx+1,ColUp,2*(fromidx-toidx),1);

            if(!warn.isEmpty()){
                table->setItem(2*toidx+1, ColInfo, new TI(warn));
            }
        }
    }
}

/**
 * @brief RailTopoTable::initUI
 * 本函数只负责初始化结构，把Table的形式做好，但不填内容。
 */
void RailTopoTable::initUI()
{
    resize(600,700);
    auto* vlay=new QVBoxLayout(this);
    auto* lab=new QLabel(tr("本界面检查并显示线路的基本拓扑（topological）信息，"
        "主要是单向站、单线区间性质。如需改动相关设置，请前往基线编辑。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    table=new QTableWidget;

    table->setColumnCount(ColMAX);

    table->setHorizontalHeaderLabels({ tr("对里程"),tr("公里标"),tr("站名"),
                                     tr("下行区间"),tr("上行区间"),tr("信息")});
    {
        int c=0;
        for (int t:{100,80,80,90,90,120}){
            table->setColumnWidth(c++,t);
        }
    }
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->verticalHeader()->hide();
    vlay->addWidget(table);
    auto* btn=new QPushButton(tr("关闭"));
    connect(btn,&QPushButton::clicked,this,&QWidget::close);
    vlay->addWidget(btn);
}
