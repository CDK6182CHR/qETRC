#include "trainlinedialog.h"

#include "model/delegate/qedelegate.h"
#include "data/diagram/trainadapter.h"
#include "data/common/qesystem.h"
#include "data/train/train.h"
#include "data/rail/railway.h"
#include "util/utilfunc.h"
#include "data/diagram/diagramoptions.h"

#include <QLabel>
#include <QSplitter>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QScroller>

TrainLineListModel::TrainLineListModel(const DiagramOptions& ops, std::shared_ptr<Train> train_, QObject *parent):
    QStandardItemModel(parent), _ops(ops), train(train_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
    tr("线路"),tr("起点"),tr("终点"),tr("方向"),tr("起标签"),tr("止标签"),
    tr("站数"),tr("里程"),tr("旅速")
                              });
    setupModel();
}



void TrainLineListModel::setupModel()
{
    //不能直接确定运行线数量。一个个往里插入算了
    using SI=QStandardItem;
    beginResetModel();
    setRowCount(0);

    int row=0;
    for(auto adp:train->adapters()){
        for(auto line: adp->lines()){
            insertRow(row);
            auto* it=new SI(adp->railway()->name());
            QVariant v;
            v.setValue(line);
            it->setData(v,qeutil::TrainLineRole);
            setItem(row,ColRailway,it);

            setItem(row,ColStart,new SI(line->firstStationName().toSingleLiteral()));
            setItem(row,ColEnd,new SI(line->lastStationName().toSingleLiteral()));
            setItem(row,ColDir,new SI(DirFunc::dirToString(line->dir())));

            it=new SI;
            it->setCheckState(qeutil::boolToCheckState(line->startLabel()));
            it->setTextAlignment(Qt::AlignCenter);
            setItem(row,ColStartLabel,it);

            it=new SI;
            it->setCheckState(qeutil::boolToCheckState(line->endLabel()));
            it->setTextAlignment(Qt::AlignCenter);
            setItem(row,ColEndLabel,it);

            setItem(row,ColStationCount,new SI(QString::number(line->count())));

            double mile=line->totalMile();
            int sec=line->runStaySecs(_ops.period_hours).first;
            double spd=mile/sec*3600.0;

            setItem(row,ColMile,new SI(QString::number(mile,'f',3)));
            setItem(row,ColSpeed,new SI(QString::number(spd,'f',3)));

            row++;
        }
    }


    endResetModel();
}

void TrainLineListModel::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(current.isValid() && previous.row()!=current.row()){
        auto t=qvariant_cast<std::shared_ptr<TrainLine>>(
             item(current.row(),ColRailway)->data(qeutil::TrainLineRole));
        emit currentTrainLineChanged(t);
    }
}

TrainLineDetailModel::TrainLineDetailModel(QObject *parent):
    QStandardItemModel(parent)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
     tr("列车站名"),tr("到点"),tr("开点"),tr("股道"),
     tr("线路站名"),tr("公里标")
                              });
}

void TrainLineDetailModel::setLine(std::shared_ptr<TrainLine> line_)
{
    line=line_;
    setupModel();
}

void TrainLineDetailModel::setupModel()
{
    beginResetModel();
    setRowCount(0);
    int row=0;
    auto p=line->stations().begin(),pr=p;
    for(;p!=line->stations().end();++p){
        if(p!=pr){
            //处理前一铺画区间。先列车后线路
            auto tp=pr->trainStation;
            for(++tp;tp!=line->train()->nullStation() &&
                tp!=p->trainStation;++tp){
                insertRow(row);
                setTrainRow(row++,tp,false);
            }
            if (tp == line->train()->nullStation()) {
                qDebug() << "TrainLineDetailModel::setupModel: WRNING: "
                    << "Unexpected terminate of Train iterator. Before station" <<
                    p->trainStation->name.toSingleLiteral() << Qt::endl;
            }

            //线路上的站
            auto rp=pr->railStation.lock()->dirAdjacent(line->dir());
            auto rcur=p->railStation.lock();
            for(;rp && rp!=rcur;rp=rp->dirAdjacent(line->dir())){
                insertRow(row);
                setRailwayRow(row++,rp,false);
            }
        }

        //处理当前站
        insertRow(row);
        setTrainRow(row,p->trainStation,true);
        setRailwayRow(row,p->railStation.lock(),true);
        row++;

        pr=p;
    }

    endResetModel();
}

void TrainLineDetailModel::setTrainRow(int row, Train::ConstStationPtr st, bool bound)
{
    //tr("列车站名"),tr("到点"),tr("开点"),tr("股道"),
    using SI=QStandardItem;
    setItem(row,ColTrainStation,new SI(st->name.toSingleLiteral()));
    setItem(row,ColArrive,new SI(st->arrive.toString(TrainTime::HMS)));
    setItem(row,ColDepart,new SI(st->depart.toString(TrainTime::HMS)));
    setItem(row,ColTrack,new SI(st->track));
    QBrush brush;
    if(bound)
        brush.setColor(Qt::blue);
    else
        brush.setColor(Qt::gray);
    for(int c=ColTrainStation;c<=ColTrack;c++){
        item(row,c)->setData(brush,Qt::ForegroundRole);
    }
}

void TrainLineDetailModel::setRailwayRow(int row, std::shared_ptr<const RailStation> st, bool bound)
{
    //线路站名，公里标
    setItem(row,ColRailStation,new QStandardItem(st->name.toSingleLiteral()));
    setItem(row,ColMile,new QStandardItem(QString::number(st->mile,'f',3)));
    QBrush brush;
    if(bound)
        brush.setColor(Qt::red);
    else
        brush.setColor(Qt::gray);
    for(int c=ColRailStation;c<=ColMile;c++){
        item(row,c)->setForeground(brush);
    }
}

TrainLineDialog::TrainLineDialog(const DiagramOptions& ops, std::shared_ptr<Train> train_, QWidget* parent):
    QDialog(parent), _ops(ops), train(train_),mdList(new TrainLineListModel(_ops, train_,this)),
    mdDetail(new TrainLineDetailModel(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("运行线一览 - %1").arg(train->trainName().full()));
    resize(800, 800);
    initUI();
}

void TrainLineDialog::initUI()
{
    auto* vlay = new QVBoxLayout;

    auto* lab = new QLabel(tr("下面列出本车次的所有运行线信息。左侧为所有运行线的基础信息，"
        "右侧为所选运行线的铺画情况。与pyETRC不同，本系统暂不支持手工定义运行线，"
        "此处仅能浏览。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    auto* sp = new QSplitter;
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    tbList = new QTableView;
    tbList->setModel(mdList);
    tbList->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tbList->setEditTriggers(QTableView::NoEditTriggers);
    tbList->setSelectionMode(QTableView::SingleSelection);
    tbList->setSelectionBehavior(QTableView::SelectRows);
    connect(tbList->selectionModel(), &QItemSelectionModel::currentRowChanged,
        mdList, &TrainLineListModel::onCurrentRowChanged);
    tbList->resizeColumnsToContents();
    sp->addWidget(tbList);
    QScroller::grabGesture(tbList,QScroller::TouchGesture);

    tbDetail = new QTableView;
    tbDetail->setModel(mdDetail);
    tbDetail->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    tbDetail->setEditTriggers(QTableView::NoEditTriggers);
    connect(mdList, &TrainLineListModel::currentTrainLineChanged,
        mdDetail, &TrainLineDetailModel::setLine);
    sp->addWidget(tbDetail);
    QScroller::grabGesture(tbDetail,QScroller::TouchGesture);

    if (mdList->rowCount()) {
        tbList->setCurrentIndex(mdList->index(0, 0));
    }
    tbDetail->resizeColumnsToContents();

    vlay->addWidget(sp);
    setLayout(vlay);
}
