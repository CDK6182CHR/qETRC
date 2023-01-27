#include "traindiffdialog.h"
#include "data/diagram/trainadapter.h"
#include "util/utilfunc.h"
#include "data/common/qesystem.h"
#include "data/diagram/diagram.h"
#include "data/train/train.h"
#include "data/rail/railway.h"

#include <cmath>
#include <QFormLayout>
#include <QTableView>
#include <QHeaderView>

#include <util/selectrailwaycombo.h>
#include <util/selecttraincombo.h>


TrainDiffModel::TrainDiffModel(Diagram &diagram_, QObject *parent):
    QStandardItemModel(parent), diagram(diagram_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
    tr("区间"),tr("里程"),tr("历时1"),tr("技速1"),tr("附加1"),
    tr("历时2"),tr("技速2"),tr("附加2")
                              });
}

void TrainDiffModel::setRailway(std::shared_ptr<Railway> railway)
{
    this->railway = railway;
    setTrain1(train1);
    setTrain2(train2);
    setupModel();
}

void TrainDiffModel::setupModel()
{
    if (!railway) {
        setRowCount(0);
        return;
    }
    using SI = QStandardItem;
    beginResetModel();
    setRowCount(0);
    int row = 0;
    for (auto p = railway->firstDownInterval(); p; p = railway->nextIntervalCirc(p)) {
        intermap_t::const_iterator itr1, itr2;
        bool flag1 = ((itr1 = map1.find(p)) != map1.cend());
        bool flag2 = ((itr2 = map2.find(p)) != map2.cend());
        int secs1 = -1, secs2 = -1;
        if (flag1||flag2) {
            // 插入本区间
            insertRow(row);
            setItem(row, ColInterval, new SI(p->toString()));
            setItem(row, ColMile, new SI(QString::number(p->mile(), 'f', 3)));
            if (flag1) {
                secs1 = itr1->second.first;
                setItem(row, ColTime1, new SI(qeutil::secsDiffToString(secs1)));
                double spd = p->mile() / secs1 * 3600;
                setItem(row, ColSpeed1, new SI(QString::number(spd, 'f', 3)));
                setItem(row, ColAppend1, new SI(itr1->second.second));
            }
            else {
                for (int c = ColTime1; c <= ColAppend1; c++)
                    setItem(row, c, new SI("-"));
            }
            if (flag2) {
                secs2 = itr2->second.first;
                setItem(row, ColTime2, new SI(qeutil::secsDiffToString(secs2)));
                double spd = p->mile() / secs2 * 3600;
                setItem(row, ColSpeed2, new SI(QString::number(spd, 'f', 3)));
                setItem(row, ColAppend2, new SI(itr2->second.second));
            }
            else {
                for (int c = ColTime2; c <= ColAppend2; c++) {
                    setItem(row, c, new SI("-"));
                }
            }
            if (flag1 && flag2 && secs1!=secs2) {
                //设置颜色..
                int alpha1 = std::round(std::fabs(secs1 - secs2) / secs1 * 200) + 55;
                int alpha2 = std::round(std::fabs(secs1 - secs2) / secs2 * 200) + 55;
                QColor color1(Qt::red), color2(Qt::blue);
                if (secs1 > secs2) {
                    std::swap(color1, color2);
                }
                color1.setAlpha(alpha1);
                color2.setAlpha(alpha2);
                for (int c = ColTime1; c <= ColAppend1; c++)
                    item(row, c)->setBackground(color1);
                for (int c = ColTime2; c <= ColAppend2; c++)
                    item(row, c)->setBackground(color2);
            }
            row++;
        }
    }
    endResetModel();
}

void TrainDiffModel::setTrainMap(std::shared_ptr<Train> train, intermap_t& imap)
{
    imap.clear();
    if (!train)
        return;
    auto adp = train->adapterFor(*railway);
    if (!adp)
        return;
    foreach(auto line, adp->lines()) {
        if (line->count() > 1) {
            auto pr = line->stations().begin();
            for (auto p = std::next(pr); p != line->stations().end(); ++p) {
                if (pr->railStation.lock()->dirAdjacent(line->dir()) 
                    == p->railStation.lock()) {
                    auto railint = pr->railStation.lock()->dirNextInterval(line->dir());
                    int secs = qeutil::secsTo(pr->trainStation->depart,
                        p->trainStation->arrive);
                    imap.emplace(railint,
                        std::make_pair(secs, line->appStringShort(pr, p)));
                }
                pr = p;
            }
        }
    }
}

void TrainDiffModel::setTrain1(std::shared_ptr<Train> train)
{
    train1 = train;
    setTrainMap(train, map1);
    setupModel();
}

void TrainDiffModel::setTrain2(std::shared_ptr<Train> train)
{
    train2 = train;
    setTrainMap(train, map2);
    setupModel();
}

TrainDiffDialog::TrainDiffDialog(Diagram& diagram_, QWidget* parent):
    QDialog(parent),diagram(diagram_),model(new TrainDiffModel(diagram,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(600, 600);
    setWindowTitle(tr("两车次运行对照"));
    initUI();
}


void TrainDiffDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* flay = new QFormLayout;

    cbRail = new SelectRailwayCombo(diagram.railCategory());
    flay->addRow(tr("线路选择"), cbRail);
    cbTrain1 = new SelectTrainCombo(diagram.trainCollection());
    flay->addRow(tr("左车次"), cbTrain1);
    cbTrain2 = new SelectTrainCombo(diagram.trainCollection());
    flay->addRow(tr("右车次"), cbTrain2);
    vlay->addLayout(flay);

    table = new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setModel(model);
    vlay->addWidget(table);

    connect(cbRail, &SelectRailwayCombo::currentRailwayChanged,
        model, &TrainDiffModel::setRailway);
    connect(cbTrain1, &SelectTrainCombo::currentTrainChanged,
        model, &TrainDiffModel::setTrain1);
    connect(cbTrain2, &SelectTrainCombo::currentTrainChanged,
        model, &TrainDiffModel::setTrain2);

    model->setRailway(cbRail->railway());
    model->setTrain1(cbTrain1->train());
    model->setTrain2(cbTrain2->train());

    connect(model, SIGNAL(modelReset()), table, SLOT(resizeColumnsToContents()));
}
