#include "railsectionevents.h"
#include <QtWidgets>
#include "util/buttongroup.hpp"
#include "util/utilfunc.h"

RailSectionEventsModel::RailSectionEventsModel(Diagram &diagram_,
                                               std::shared_ptr<Railway> rail,
                                               QObject *parent):
    QStandardItemModel(parent),diagram(diagram_),railway(rail)
{
    setHorizontalHeaderLabels({
      tr("车次"),tr("时间"),tr("类型"),tr("方向"),tr("始发"),tr("终到"),
                                  tr("车底"),tr("担当")
                              });
    setColumnCount(ColMAX);

}

void RailSectionEventsModel::setY(double y)
{
    this->y=y;
    setupModel();
}

void RailSectionEventsModel::setupModel()
{
    using SI = QStandardItem;
    lst = diagram.sectionEvents(railway, y);

    beginResetModel();
    setRowCount(static_cast<int>(lst.size()));
    for (int i = 0; i < (int)lst.size(); i++) {
        const auto& p = lst.at(i);
        auto line = p.first;
        auto train = line->train();
        setItem(i, ColTrainName, new SI(train->trainName().full()));
        setItem(i, ColTime, new SI(p.second.toString("hh:mm:ss")));
        setItem(i, ColType, new SI(train->type()->name()));
        setItem(i, ColDir, new SI(DirFunc::dirToString(line->dir())));
        setItem(i, ColStarting, new SI(train->starting().toSingleLiteral()));
        setItem(i, ColTerminal, new SI(train->terminal().toSingleLiteral()));
        if (train->hasRouting()) {
            auto rt = train->routing().lock();
            setItem(i, ColModel, new SI(rt->model()));
            setItem(i, ColOwner, new SI(rt->owner()));
        }
        else {
            setItem(i, ColModel, new SI("-"));
            setItem(i, ColOwner, new SI("-"));
        }
    }
    endResetModel();
}



RailSectionEventsDialog::RailSectionEventsDialog(Diagram &diagram_,
                   std::shared_ptr<Railway> railway_, QWidget *parent):
    QDialog(parent),diagram(diagram_),railway(railway_),
    model(new RailSectionEventsModel(diagram_,railway_,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("断面事件表 - %1").arg(railway->name()));
    resize(800,800);
    initUI();
}

void RailSectionEventsDialog::initUI()
{
    auto* vlay=new QVBoxLayout;
    auto* hlay=new QHBoxLayout;

    auto* lab=new QLabel(tr("请输入要查询事件表的公里标，然后点击确定。"
        "注意此功能仅考虑输入的公里标位于站间而不是站内，所有事件默认为通过。如果所给公里标位于站内，"
        "将理解为该站站前的区间。如果需要查看车站事件表，请使用[车站事件表]功能。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    hlay->addWidget(new QLabel(tr("公里标")));
    auto* sp=new QDoubleSpinBox;
    sp->setRange(-100000, 100000);
    sp->setDecimals(3);
    spMile=sp;
    hlay->addWidget(sp);
    hlay->addStretch(1);
    auto* btn=new QPushButton(tr("确定"));
    connect(btn,SIGNAL(clicked()),this,SLOT(updateData()));
    hlay->addWidget(btn);
    vlay->addLayout(hlay);

    hlay=new QHBoxLayout;
    auto* ed=new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    edDownIt=ed;
    hlay->addWidget(new QLabel(tr("下行区间")));
    hlay->addWidget(ed);
    hlay->addSpacing(1);

    ed=new QLineEdit;
    ed->setFocusPolicy(Qt::NoFocus);
    edUpIt=ed;
    hlay->addWidget(new QLabel(tr("上行区间")));
    hlay->addWidget(ed);
    vlay->addLayout(hlay);

    table=new QTableView;
    table->setModel(model);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);

    connect(table->horizontalHeader(),SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)),
            table,SLOT(sortByColumn(int,Qt::SortOrder)));
    table->horizontalHeader()->setSortIndicatorShown(true);
    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"导出CSV","关闭"});
    g->connectAll(SIGNAL(clicked()),this,{
                      SLOT(toCsv()),SLOT(close())
                  });
    vlay->addLayout(g);
    setLayout(vlay);
}

void RailSectionEventsDialog::updateData()
{
    double mile=spMile->value();
    if(railway->empty() || mile< railway->stationByIndex(0)->mile ||
            mile>railway->totalMile()) {
        QMessageBox::warning(this,tr("错误"),
                             tr("里程标错误：所给里程标不在本线的合法里程范围内"));
        return;
    }
    auto r=railway->getSectionInfo(mile);
    if(r==std::nullopt){
        QMessageBox::warning(this,tr("错误"),
                             tr("里程标错误：根据所给里程找不到区间信息"));
        return;
    }
    double y=std::get<0>(r.value());
    auto itDown=std::get<1>(r.value());
    auto itUp=std::get<2>(r.value());
    if(itDown){
        edDownIt->setText(itDown->toString());
    }else{
        edDownIt->setText(tr("非法区间"));
    }

    if(itUp){
        edUpIt->setText(itUp->toString());
    }else{
        edUpIt->setText(tr("非法区间"));
    }

    model->setY(y);
    table->resizeColumnsToContents();
}

void RailSectionEventsDialog::toCsv()
{
    QString r=tr("%1断面事件表").arg(railway->name());
    qeutil::exportTableToCsv(model,this,r);
}
