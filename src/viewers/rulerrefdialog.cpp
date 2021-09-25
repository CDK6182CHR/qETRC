#include "rulerrefdialog.h"

#include <cmath>
#include <QComboBox>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <initializer_list>
#include "data/diagram/trainadapter.h"
#include "util/utilfunc.h"
#include "data/common/qesystem.h"
#include "data/train/train.h"
#include "data/rail/ruler.h"
#include "data/rail/rulernode.h"

RulerRefModel::RulerRefModel(std::shared_ptr<Train> train_, QObject *parent):
    QStandardItemModel(parent),train(train_)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({
       tr("区间"),tr("通通"),tr("起"),tr("停"),tr("标准"), tr("实际"),tr("里程"),
                                  tr("技速"),tr("附加"),tr("差时")
        });
    setRailwayIndex(0);
}

void RulerRefModel::setupModel()
{
    using SI = QStandardItem;
    beginResetModel();
    int row = 0;
    setRowCount(0);
    for (auto line : adp->lines()) {
        setRowCount(row + static_cast<int>( line->count()) - 1);
        auto pr = line->stations().begin();
        auto p = std::next(pr);
        for (; p != line->stations().end(); pr = p, ++p,++row) {
            QString its = tr("%1->%2").arg(pr->trainStation->name.toSingleLiteral())
                .arg(p->trainStation->name.toSingleLiteral());
            setItem(row, ColInterval, new SI(its));

            //先填写运行数据部分
            int secs = qeutil::secsTo(pr->trainStation->depart, p->trainStation->arrive);
            double mile = std::fabs(p->railStation.lock()->mile -
                pr->railStation.lock()->mile);
            double spd = mile / secs * 3600.0;

            setItem(row, ColReal, new SI(qeutil::secsDiffToString(secs)));
            setItem(row, ColMile, new SI(QString::number(mile, 'f', 3)));
            setItem(row, ColSpeed, new SI(QString::number(spd, 'f', 3)));

            QString append;
            bool with_start = (pr->trainStation->isStopped() || train->isStartingStation(&*pr));
            bool with_stop = (p->trainStation->isStopped() || train->isTerminalStation(&*p));
            if (with_start)
                append += tr("起");
            if (with_stop)
                append += tr("停");
            setItem(row, ColAppend, new SI(append));

            QColor color(Qt::transparent);     //背景颜色  默认无

            if (ruler) {
                bool valid = true;     //必须所请求的区间、起停数据皆为有效，最终数据才有效
                do {
                    int interval = ruler->totalInterval(pr->railStation.lock(), p->railStation.lock(),
                        line->dir());
                    if (interval <= 0) {
                        valid = false; break;
                    }
                    auto ndstart = ruler->dirNextNode(pr->railStation.lock(), line->dir());
                    auto ndstop = ruler->dirPrevNode(p->railStation.lock(), line->dir());
                    if (!ndstart || !ndstop) {
                        valid = false; break;
                    }
                    int stdtm = interval;
                    if (with_start)
                        stdtm += ndstart->start;
                    if (with_stop)
                        stdtm += ndstop->stop;

                    //现在获得了正确的标准事件
                    setItem(row, ColPass, new SI(qeutil::secsDiffToString(interval)));
                    setItem(row, ColStart, new SI(qeutil::secsDiffToString(ndstart->start)));
                    setItem(row, ColStop, new SI(qeutil::secsDiffToString(ndstop->stop)));
                    setItem(row, ColStd, new SI(qeutil::secsDiffToString(stdtm)));

                    int dif = secs - stdtm;
                    setItem(row, ColDiff, new SI(qeutil::secsDiffToString(dif)));

                    //设置颜色
                    if (secs != stdtm) {
                        double rate = static_cast<double>(secs - stdtm) / stdtm;
                        if (secs > stdtm) {
                            color = Qt::blue;
                        }
                        else {
                            color = Qt::red;
                        }
                        color.setAlpha(std::min(std::fabs(rate) * 200, 200.0) + 55);
                    }
                    
                } while (false);

                if (!valid) {
                    //设置空白占位字符
                    auto lst = { ColPass,ColStart,ColStop,ColStd,ColDiff };
                    for (auto c : lst) {
                        setItem(row, c, new SI("NA"));
                    }
                }

                for (int c = 0; c < columnCount(); c++) {
                    item(row, c)->setBackground(color);
                }

            }
        }
    }
    endResetModel();
}

void RulerRefModel::setRailwayIndex(int i)
{
    if (i < 0 || i >= train->adapters().size()) {
        adp.reset();
        return;
    }
    adp = train->adapters().at(i);
    ruler.reset();
    setupModel();
}

void RulerRefModel::setRulerIndex(int i)
{
    ruler.reset();
    if (adp) {
        auto rail = adp->railway();
        if (i > 0 && i <= rail->rulers().size()) {
            ruler = rail->getRuler(i - 1);
        }
    }
    setupModel();
}

RulerRefDialog::RulerRefDialog(std::shared_ptr<Train> train_, QWidget* parent):
    QDialog(parent),train(train_),model(new RulerRefModel(train_,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("标尺对照 - %1").arg(train->trainName().full()));
    resize(800, 800);
    initUI();
}

void RulerRefDialog::initUI()
{
    auto* vlay = new QVBoxLayout;
    auto* hlay = new QHBoxLayout;
    cbRail = new QComboBox;
    cbRail->setEditable(false);
    
    cbRuler = new QComboBox;
    connect(cbRail, SIGNAL(currentIndexChanged(int)), this,
        SLOT(onRailIndexChanged(int)));
    connect(cbRuler, SIGNAL(currentIndexChanged(int)), model,
        SLOT(setRulerIndex(int)));

    for (auto t : train->adapters()) {
        cbRail->addItem(t->railway()->name());
    }
    
    hlay->addWidget(cbRail);
    hlay->addWidget(cbRuler);
    vlay->addLayout(hlay);

    table = new QTableView;
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table->setModel(model);
    table->resizeColumnsToContents();
    vlay->addWidget(table);

    connect(model, SIGNAL(modelReset()), table, SLOT(resizeColumnsToContents()));
    setLayout(vlay);
}

void RulerRefDialog::onRailIndexChanged(int i)
{
    updating = true;

    model->setRailwayIndex(i);

    auto adp = train->adapters().at(i);

    cbRuler->clear();
    cbRuler->addItem(tr("(空)"));
    for (auto t : adp->railway()->rulers()) {
        cbRuler->addItem(t->name());
    }

    updating = false;
}
