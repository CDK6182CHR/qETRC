#include "selectrailstationdialog.h"
#include <QtWidgets>

#include "util/buttongroup.hpp"
#include "data/diagram/diagram.h"
#include "model/delegate/generaldoublespindelegate.h"

SelectRailStationDialog::SelectRailStationDialog(std::shared_ptr<Railway> rail,
                                                 QWidget *parent):
    QDialog(parent), railway(rail), model(new RailStationModel(rail,true,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(500,600);
    initUI();
}

std::shared_ptr<RailStation> SelectRailStationDialog::getStation(
    std::shared_ptr<Railway> rail, QWidget* parent)
{
    auto* dlg = new SelectRailStationDialog(rail, parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose, false);
    int flag = dlg->exec();
    std::shared_ptr<RailStation> res{};
    if (flag) {
        res = dlg->station();
    }
    dlg->setParent(nullptr);
    delete dlg;
    return res;
}

void SelectRailStationDialog::initUI()
{
    setWindowTitle(tr("选择车站"));
    auto* vlay=new QVBoxLayout;
    auto* lab=new QLabel(tr("当前基线：")+railway->name());
    vlay->addWidget(lab);
    lab=new QLabel(tr("请在下表中选择一个车站，双击或者点击“确定”提交。"));
    vlay->addWidget(lab);

    table=new QTableView;
    table->setModel(model);
    table->setItemDelegateForColumn(RailStationModel::ColMile,
        new GeneralDoubleSpinDelegate(this));
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->resizeColumnsToContents();
    table->setSelectionBehavior(QTableView::SelectRows);
    table->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    connect(table,&QTableView::doubleClicked,
            this,&SelectRailStationDialog::onItemDoubleClicked);
    vlay->addWidget(table);

    auto* g=new ButtonGroup<2>({"确定","取消"});
    vlay->addLayout(g);
    g->connectAll(SIGNAL(clicked()),this,{SLOT(actApply()),SLOT(close())});

    setLayout(vlay);
}

void SelectRailStationDialog::actApply()
{
    const auto& idx=table->currentIndex();
    if(!idx.isValid()){
        QMessageBox::warning(this,tr("错误"),tr("请先选择一个车站！"));
        return;
    }
    _station = railway->stations().at(idx.row());
    emit stationSelected(_station);
    done(QDialog::Accepted);
}

void SelectRailStationDialog::onItemDoubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        _station = railway->stations().at(index.row());
        emit stationSelected(_station);
        done(QDialog::Accepted);
    }
}
