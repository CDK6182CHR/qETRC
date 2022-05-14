#include "readrulerpageinterval.h"

#ifndef QETRC_MOBILE_2

#include "util/buttongroup.hpp"

#include <QtWidgets>

#include "data/rail/railway.h"

ReadRulerPageInterval::ReadRulerPageInterval(RailCategory& cat_, QWidget* parent) :
    QWizardPage(parent), cat(cat_)
{
    initUI();
}

bool ReadRulerPageInterval::validatePage()
{
    if (!_railway) {
        QMessageBox::warning(this, tr("错误"), tr("请先选择线路。"
            "出现这个警告的原因可能是当前运行图没有线路。"));
        return false;
    }
    _ruler = cbRuler->ruler();

    railints.clear();
    int row = 0;
    for (auto p = _railway->firstDownInterval(); p; p = p->nextInterval()) {
        if (lsDown->selectionModel()->isRowSelected(row)) {
            railints.push_back(p);
        }
        row++;
    }

    row = 0;
    for (auto p = _railway->firstUpInterval(); p; p = p->nextInterval()) {
        if (lsUp->selectionModel()->isRowSelected(row)) {
            railints.push_back(p);
        }
        row++;
    }

    if (railints.empty()) {
        QMessageBox::warning(this, tr("错误"), tr("请选择至少一个区间！"));
        return false;
    }

    return true;
}

void ReadRulerPageInterval::initUI()
{
    setTitle(tr("选择线路、标尺和区间"));
    setSubTitle(tr("请选择一个标尺，或者新建一个标尺。\n新读取的标尺数据将放入这个标尺，"
        "并无条件覆盖既有数据。"));
    auto* vlay = new QVBoxLayout(this);

    auto* flay = new QFormLayout;
    cbRuler = new RailRulerCombo(cat, tr("(新建标尺)"));
    flay->addRow(tr("线路、标尺"), cbRuler);
    vlay->addLayout(flay);

    auto* lab = new QLabel(tr("请在下表中选择所有要计算标尺的区间。"));
    vlay->addWidget(lab);

    auto* g = new ButtonGroup<4>({ "全选","全不选","全选","全不选" });
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()), this, {
        SLOT(selectAllDown()),SLOT(deselectAllDown()),
        SLOT(selectAllUp()),SLOT(deselectAllUp())
        });

    auto* hlay = new QHBoxLayout;
    lsDown = new QListView;
    lsDown->setSelectionMode(QListView::MultiSelection);
    lsUp = new QListView;
    lsUp->setSelectionMode(QListView::MultiSelection);
    mdDown = new QStandardItemModel;
    mdUp = new QStandardItemModel;
    lsDown->setModel(mdDown);
    lsUp->setModel(mdUp);
    hlay->addWidget(lsDown);
    hlay->addWidget(lsUp);
    vlay->addLayout(hlay);

    connect(cbRuler, &RailRulerCombo::railwayChagned,
        this, &ReadRulerPageInterval::setRailway);
    setRailway(cbRuler->railway());
}

void ReadRulerPageInterval::selectAllDown()
{
    lsDown->selectionModel()->select(QItemSelection(mdDown->index(0, 0),
        mdDown->index(mdDown->rowCount() - 1, 0)), QItemSelectionModel::Select);
}

void ReadRulerPageInterval::deselectAllDown()
{
    lsDown->selectionModel()->select(QItemSelection(mdDown->index(0, 0),
        mdDown->index(mdDown->rowCount() - 1, 0)), QItemSelectionModel::Deselect);
}

void ReadRulerPageInterval::selectAllUp()
{
    lsUp->selectionModel()->select(QItemSelection(mdUp->index(0, 0),
        mdUp->index(mdDown->rowCount() - 1, 0)), QItemSelectionModel::Select);
}

void ReadRulerPageInterval::deselectAllUp()
{
    lsUp->selectionModel()->select(QItemSelection(mdUp->index(0, 0),
        mdUp->index(mdDown->rowCount() - 1, 0)), QItemSelectionModel::Deselect);
}

void ReadRulerPageInterval::setRailway(std::shared_ptr<Railway> railway)
{
    if (!railway)
        return;
    using SI = QStandardItem;
    _railway = railway;
    //更新model
    mdDown->setRowCount(railway->stationCount());
    int row = 0;
    for (auto p = railway->firstDownInterval(); p; p = p->nextInterval()) {
        mdDown->setItem(row++, new SI(p->toString()));
    }
    mdDown->setRowCount(row);

    mdUp->setRowCount(railway->stationCount());
    row = 0;
    for (auto p = railway->firstUpInterval(); p; p = p->nextInterval()) {
        mdUp->setItem(row++, new SI(p->toString()));
    }
    mdUp->setRowCount(row);
}

#endif
