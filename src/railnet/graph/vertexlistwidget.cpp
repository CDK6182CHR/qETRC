#include "vertexlistwidget.h"
#include "model/delegate/qedelegate.h"
#include "util/utilfunc.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <data/common/qesystem.h>


VertexListModel::VertexListModel(const RailNet &net, QObject *parent):
    QStandardItemModel(parent),net(net)
{
    setColumnCount(ColMAX);
    setHorizontalHeaderLabels({tr("站名"),tr("入度"),tr("出度"),tr("等级"),
                               tr("办客"),tr("办货")});
    setupModel();
}

void VertexListModel::refreshData()
{
    setupModel();
}

std::shared_ptr<RailNet::vertex> VertexListModel::vertexForRow(int row) const
{
    return qvariant_cast<std::shared_ptr<RailNet::vertex>>(
              item(row,ColName)->data(qeutil::GraphVertexRole));
}

int VertexListModel::searchStation(const QString &name)
{
    for(int i=0;i<rowCount();i++){
        if(item(i,ColName)->text()==name)
            return i;
    }
    return -1;
}

void VertexListModel::setupModel()
{
    setRowCount(net.size());
    using SI=QStandardItem;

    int row=0;
    for(auto p=net.vertices().begin();p!=net.vertices().end();++p){
        const auto& d=p->second->data;
        auto* it=new SI(d.name.toSingleLiteral());
        it->setData(QVariant::fromValue(p->second), qeutil::GraphVertexRole);
        setItem(row,ColName,it);

        it=new SI;
        it->setData(p->second->in_degree(),Qt::EditRole);
        setItem(row,ColInDeg,it);
        it=new SI;
        it->setData(p->second->out_degree(),Qt::EditRole);
        setItem(row,ColOutDeg,it);
        it=new SI;
        it->setData(d.level,Qt::EditRole);
        setItem(row,ColLevel,it);

        it=new SI;
        it->setCheckState(qeutil::boolToCheckState(d.passenger));
        it->setCheckable(false);
        setItem(row,ColPassenger,it);

        it=new SI;
        it->setCheckState(qeutil::boolToCheckState(d.freight));
        it->setCheckable(false);
        setItem(row,ColFreight,it);

        row++;
    }
}



VertexListWidget::VertexListWidget(const RailNet &net, QWidget *parent):
    QWidget(parent), model(new VertexListModel(net,this))
{
    initUI();
}

void VertexListWidget::refreshData()
{
    model->refreshData();
}

void VertexListWidget::initUI()
{
    auto* vlay=new QVBoxLayout(this);
    auto* hlay=new QHBoxLayout;
    edSearch=new QLineEdit;
    hlay->addWidget(edSearch);
    auto* btn=new QPushButton(tr("搜索"));
    hlay->addWidget(btn);
    connect(btn,&QPushButton::clicked,this,&VertexListWidget::searchStation);
    vlay->addLayout(hlay);

    table=new QTableView;
    table->verticalHeader()->setDefaultSectionSize(SystemJson::get().table_row_height);
    table->setEditTriggers(QTableView::NoEditTriggers);
    table->setModel(model);
    connect(table->selectionModel(),&QItemSelectionModel::currentRowChanged,
            this,&VertexListWidget::onRowChanged);
    table->horizontalHeader()->setSortIndicatorShown(true);
    connect(table->horizontalHeader(),
            qOverload<int,Qt::SortOrder>(&QHeaderView::sortIndicatorChanged),
            table,
            qOverload<int,Qt::SortOrder>(&QTableView::sortByColumn));
    table->resizeColumnsToContents();
    vlay->addWidget(table);
}

void VertexListWidget::searchStation()
{
    int idx=model->searchStation(edSearch->text());
    if(idx==-1){
        QMessageBox::warning(this,tr("错误"),tr("无符合条件车站"));
    }else{
        table->setCurrentIndex(model->index(idx,0));
    }
}

void VertexListWidget::onRowChanged(const QModelIndex &idx)
{
    if (!idx.isValid())return;
    emit currentVertexChanged(model->vertexForRow(idx.row()));
}
