#include "conflictdialog.h"
#include "util/utilfunc.h"
#include "data/common/qesystem.h"
#include "data/diagram/diagram.h"
#include "data/train/train.h"
#include "data/train/traintype.h"

#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>


ConflictModel::ConflictModel(Diagram &diagram_, QObject *parent):
    QStandardItemModel(parent),diagram(diagram_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
    tr("方向"),tr("车次"),tr("时刻"),tr("事件"),tr("位置"),tr("类型")
        });
}

void ConflictModel::setupModel(std::shared_ptr<Railway> railway_, 
    std::shared_ptr<const RailStation> station_, const QTime& arrive_, const QTime& depart_)
{
    //显示条件：到达前20分钟，出发后60分钟；
    //标红条件：到达前10分钟，出发后10分钟。
    railway = railway_;
    station = station_;
    arrive = arrive_;
    depart = depart_;
    if (!railway || !station) {
        setRowCount(0);
        return;
    }

    QTime showLeft = arrive.addSecs(-1200), showRight = depart.addSecs(3600);
    QTime redLeft = arrive.addSecs(-600), redRight = depart.addSecs(600);

    auto lst = diagram.stationEvents(railway, station);

    beginResetModel();
    using SI = QStandardItem;
    setRowCount(0);
    int row = 0;
    for (auto p = lst.begin(); p != lst.end(); ++p) {
        auto line = (*p)->line;
        auto train = line->train();
        const auto& ev = **p;
        if (qeutil::timeInRange(showLeft, showRight, ev.time)) {
            insertRow(row);

            setItem(row, ColDir, new SI(DirFunc::dirToString(line->dir())));
            setItem(row, ColTrainName, new SI(train->trainName().full()));
            setItem(row, ColTime, new SI(ev.time.toString("hh:mm:ss")));
            setItem(row, ColEvent, new SI(qeutil::eventTypeString(ev.type)));
            setItem(row, ColPos, new SI(ev.posString()));
            setItem(row, ColType, new SI(train->type()->name()));

            if (qeutil::timeInRange(redLeft, redRight, ev.time)) {
                for (int c = 0; c < ColMAX; c++) {
                    item(row, c)->setData(QColor(Qt::red), Qt::ForegroundRole);
                }
            }

            row++;
        }
    }
    endResetModel();
}



ConflictDialog::ConflictDialog(Diagram &diagram_, QWidget *parent):
    QDialog(parent), diagram(diagram_),model(new ConflictModel(diagram_,this))
{
    resize(500, 500);
    initUI();
}

void ConflictDialog::setData(std::shared_ptr<Railway> railway_, 
    std::shared_ptr<const RailStation> station_, const QTime& arrive_, const QTime& depart_)
{
    railway = railway_;
    station = station_;
    arrive = arrive_;
    depart = depart_;
    model->setupModel(railway, station, arrive, depart);
    table->resizeColumnsToContents();
    edName->setText(station->name.toSingleLiteral());
    edArrive->setText(arrive.toString(QStringLiteral("hh:mm:ss")));
    edDepart->setText(depart.toString(QStringLiteral("hh:mm:ss")));
    setWindowTitle(tr("冲突检查 - %1").arg(station->name.toSingleLiteral()));
}

void ConflictDialog::initUI()
{
    auto* vlay = new QVBoxLayout(this);
    auto* flay = new QFormLayout;

    auto* lab = new QLabel(tr("现在显示指定排图车站所设置到达时刻前20分钟至出发时刻后60分钟"
        "范围内的事件表。到达、出发前后各10分钟范围内的事件标红显示。"));
    lab->setWordWrap(true);
    vlay->addWidget(lab);

    edName = new QLineEdit;
    edName->setFocusPolicy(Qt::NoFocus);
    flay->addRow(tr("当前车站"), edName);

    auto* hlay = new QHBoxLayout;
    edArrive = new QLineEdit;
    edArrive->setFocusPolicy(Qt::NoFocus);
    hlay->addWidget(edArrive);
    hlay->addStretch(1);
    hlay->addWidget(new QLabel("-"));
    hlay->addStretch(1);
    edDepart = new QLineEdit;
    edDepart->setFocusPolicy(Qt::NoFocus);
    hlay->addWidget(edDepart);
    flay->addRow(tr("到开时刻"), hlay);
    vlay->addLayout(flay);

    table = new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->horizontalHeader()->setSortIndicatorShown(true);
    connect(table->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
        table, SLOT(sortByColumn(int, Qt::SortOrder)));
    table->setModel(model);
    vlay->addWidget(table);
}

