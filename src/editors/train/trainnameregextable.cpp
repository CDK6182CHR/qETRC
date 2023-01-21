#include "trainnameregextable.h"
#include "model/general/qemoveablemodel.h"
#include <QTableView>
#include <QHeaderView>
#include "data/common/qesystem.h"


TrainNameRegexTable::TrainNameRegexTable(TrainCollection &coll_, QWidget *parent):
    QEControlledTable(parent),coll(coll_),model(new QEMoveableModel(this))
{
    initUI();
}

QVector<QRegExp> TrainNameRegexTable::names() const
{
    QVector<QRegExp> res;
    for(int i=0;i<model->rowCount();i++){
        if(auto* it=model->item(i,0)){
            if(!it->text().isEmpty()){
                QRegExp re(it->text());
                res.push_back(re);
            }
        }
    }
    return res;
}

void TrainNameRegexTable::initUI()
{
    model->setColumnCount(1);
    model->setHorizontalHeaderLabels({tr("车次正则")});

    table()->verticalHeader()->setDefaultSectionSize(SystemJson::instance.table_row_height);
    table()->setEditTriggers(QTableView::AllEditTriggers);
    table()->setModel(model);
}

void TrainNameRegexTable::refreshData(const QVector<QRegExp> &_names)
{
    model->setRowCount(_names.size());
    for(int i=0;i<model->rowCount();i++){
        model->setItem(i,new QStandardItem(_names.at(i).pattern()));
    }
}

void TrainNameRegexTable::clearNames()
{
    refreshData({});
}
