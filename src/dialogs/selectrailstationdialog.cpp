#include "selectrailstationdialog.h"
#include <QtWidgets>

#include "util/buttongroup.hpp"
#include "data/diagram/diagram.h"

SelectRailStationDialog::SelectRailStationDialog(std::shared_ptr<Railway> rail,
                                                 QWidget *parent):
    QDialog(parent), railway(rail), model(new RailStationModel(rail,true,this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(500,600);
    initUI();
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
    emit stationSelected(railway->stations().at(idx.row()));
    done(QDialog::Accepted);
}

void SelectRailStationDialog::onItemDoubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        emit stationSelected(railway->stations().at(index.row()));
        done(QDialog::Accepted);
    }
}
